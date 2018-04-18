#include "standard_header.hpp"
#include "image_resource.hpp"

ImageResource::ImageResource(ImageResource&& other) noexcept
	: Resource(std::move(other))
	, m_vkImage(std::move(other.m_vkImage))
	, m_vkImageView(std::move(other.m_vkImageView))
	, m_format(other.m_format)
	, m_vkExtent(other.m_vkExtent)
	, m_device(other.m_device)
{
	other.m_format = Format();
	other.m_vkExtent = vk::Extent3D();
}

ImageResource::~ImageResource()
{
	// HACK: If size is zero then memory was externally allocated
	if (m_size) {
		m_vkDevice->destroyImage(m_vkImage);
	}
}

void ImageResource::upload(const std::vector<char>& data)
{
	//Transition image so host can upload
	auto& commandBuffer = m_device.m_internalCommandBuffer;
	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer->begin(beginInfo);
	//TODO: don't assume Image is eUndefined at all times
	transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal>(commandBuffer);
	

	//Do the actual uploading
	//TODO: use BufferResource? -AM
	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.setSize(data.size())
		.setUsage(vk::BufferUsageFlagBits::eTransferSrc);

	auto buffer = m_vkDevice->createBufferUnique(bufferInfo);

	auto memoryRequirements = m_vkDevice->getBufferMemoryRequirements(*buffer);
	auto memoryType = findMemoryType(m_device.m_vkPhysicalDevice, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	vk::MemoryAllocateInfo allocateInfo = {};
	allocateInfo.setAllocationSize(memoryRequirements.size)
		.setMemoryTypeIndex(memoryType);

	auto memory = m_vkDevice->allocateMemoryUnique(allocateInfo);

	m_device.m_vkDevice->bindBufferMemory(*buffer, *memory, 0);
	auto map = m_vkDevice->mapMemory(*memory, 0, VK_WHOLE_SIZE);
	memcpy(map, data.data(), data.size());
	m_vkDevice->unmapMemory(*memory);
	
	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{ 0,0,0 };
	region.imageExtent = m_vkExtent;	//TODO: what if texture image is smaller/larger than ImageResource? -AM

	commandBuffer->copyBufferToImage(*buffer, m_vkImage, vk::ImageLayout::eTransferDstOptimal, { region });

	//Transition so shader can read
	
	transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal>(commandBuffer);
	commandBuffer->end();

	//execute everything
	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*commandBuffer);

	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
	m_device.m_vkInternalQueue.waitIdle();

	//cleanup
	commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}

std::vector<char> ImageResource::download()
{

	//Transition image so host can upload
	auto& commandBuffer = m_device.m_internalCommandBuffer;
	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer->begin(beginInfo);
	transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal>(commandBuffer);


	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.setSize(m_size)
		.setUsage(vk::BufferUsageFlagBits::eTransferDst);

	auto buffer = m_vkDevice->createBufferUnique(bufferInfo);

	auto memoryRequirements = m_vkDevice->getBufferMemoryRequirements(*buffer);
	auto memoryType = findMemoryType(m_device.m_vkPhysicalDevice, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
	vk::MemoryAllocateInfo allocateInfo = {};
	allocateInfo.setAllocationSize(memoryRequirements.size)
		.setMemoryTypeIndex(memoryType);

	auto memory = m_vkDevice->allocateMemoryUnique(allocateInfo);

	m_device.m_vkDevice->bindBufferMemory(*buffer, *memory, 0);

	vk::BufferImageCopy region;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{ 0,0,0 };
	region.imageExtent = m_vkExtent;	//TODO: what if texture image is smaller/larger than ImageResource? -AM

	commandBuffer->copyImageToBuffer(m_vkImage, vk::ImageLayout::eTransferSrcOptimal, *buffer, { region });

	commandBuffer->end();

	//execute everything
	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*commandBuffer);

	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
	m_device.m_vkInternalQueue.waitIdle();

	//cleanup
	commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	//TODO: transfer image to something else? -AM

	std::vector<char> result(m_size);
	auto mappedMemory = m_vkDevice->mapMemory(*memory, 0, VK_WHOLE_SIZE);
	memcpy(result.data(), mappedMemory, m_size);
	m_vkDevice->unmapMemory(*memory);
	return result;
}

void ImageResource::destroy()
{
}

ImageResource ImageResource::createDepthResource(
	const Device& device, 
	vk::Extent3D extent,
	const std::vector<Format>& formatCandidates)
{
	auto format = findSupportedFormat(
		device.m_vkPhysicalDevice, 
		formatCandidates, 
		vk::ImageTiling::eOptimal, 
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto image = device.m_vkDevice->createImage(vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setExtent(extent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setSamples(vk::SampleCountFlagBits::e1));

	auto memoryRequirements = device.m_vkDevice->getImageMemoryRequirements(image);

	return ImageResource(
		image, 
		device,
		vk::ImageAspectFlagBits::eDepth, 
		format,
		extent,
		memoryRequirements);
}

ImageResource ImageResource::createColorResource(
	vk::Image image, 
	const Device& device, 
	Format format,
	vk::Extent3D extent)
{
	return ImageResource(image, device, format, extent);
}

// Allocates memory to the image and creates an image view to the provided image
ImageResource::ImageResource(
	vk::Image& image,
	const Device& device,
	vk::ImageAspectFlags aspectFlags, 
	Format format, 
	vk::Extent3D extent,
	vk::MemoryRequirements memoryRequirements) 
		: Resource(device.m_vkPhysicalDevice, device.m_vkDevice, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible , memoryRequirements)
		, m_vkImage(image)
		, m_format(format)
		, m_vkExtent(extent)
		, m_device(device)
{
	m_vkDevice->bindImageMemory(m_vkImage, *m_vkMemory, 0);

	createImageView(m_vkDevice, aspectFlags);

	

	//TODO: transition image (via command buffers)
}

// Does NOT allocate memory, this is assumed to already be allocated; but does create a VkImageView.
ImageResource::ImageResource(vk::Image& image, const Device& device, Format format, vk::Extent3D extent)
	: Resource(device.m_vkDevice)
	, m_vkImage(std::move(image))
	, m_format(format)
	, m_vkExtent(extent)
	, m_device(device)
{
	createImageView(m_vkDevice);
}

Format ImageResource::findSupportedFormat(
	const vk::PhysicalDevice & physicalDevice, 
	const std::vector<Format>& candidateFormats, 
	vk::ImageTiling tiling, 
	vk::FormatFeatureFlags features)
{
	for (auto candidate : candidateFormats) {
		auto properties = physicalDevice.getFormatProperties(candidate);

		if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features
			|| tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
		{
			return candidate;
		}
	}

	PAPAGO_ERROR("failed to find supported format");
}

void ImageResource::createImageView(const vk::UniqueDevice &device, vk::ImageAspectFlags aspectFlags)
{
	m_vkImageView = device->createImageViewUnique(vk::ImageViewCreateInfo()
		.setImage(m_vkImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_format)
		.setSubresourceRange(vk::ImageSubresourceRange()
			.setAspectMask(aspectFlags)
			.setLevelCount(1)
			.setLayerCount(1))
	);
}

vk::UniqueFramebuffer & ImageResource::createFramebuffer(vk::RenderPass & renderPass)
{
	vk::FramebufferCreateInfo fboCreate;
	fboCreate.setAttachmentCount(1)
		.setPAttachments(&m_vkImageView.get())
		.setWidth(m_vkExtent.width)
		.setHeight(m_vkExtent.height)
		.setLayers(1) //TODO: <--- make setable? -AM
		.setRenderPass(static_cast<vk::RenderPass>(renderPass));

	m_vkFramebuffer = m_vkDevice->createFramebufferUnique(fboCreate);

	return m_vkFramebuffer;
}
