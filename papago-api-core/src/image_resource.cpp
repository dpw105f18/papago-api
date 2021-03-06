#include "standard_header.hpp"
#include "image_resource.hpp"
#include "device.hpp"
#include "ibuffer_resource.hpp"
#include "image_resource.impl"

ImageResource::ImageResource(ImageResource&& other) noexcept
	: Resource(std::move(other))
	, m_vkImage(std::move(other.m_vkImage))
	, m_vkImageView(std::move(other.m_vkImageView))
	, m_format(other.m_format)
	, m_vkExtent(other.m_vkExtent)
	, m_device(other.m_device)
	, m_vkAspectFlags(other.m_vkAspectFlags)
{
	// m_vkImage isn't automatically set to a null handle when moved
	other.m_vkImage = vk::Image();
	other.m_format = vk::Format();
	other.m_vkExtent = vk::Extent3D();
}

ImageResource::~ImageResource()
{
	// HACK: If size is zero then memory was externally allocated
	if (m_size && m_vkImage) {
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
	


	transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal>(commandBuffer);
	

	//Do the actual uploading
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

	std::vector<vk::BufferImageCopy> regions;
	
	if (m_vkAspectFlags & vk::ImageAspectFlagBits::eColor) {
		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = m_vkAspectFlags;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D{ 0,0,0 };
		region.imageExtent = m_vkExtent;

		regions.push_back(region);
	}
	else if (m_vkAspectFlags & vk::ImageAspectFlagBits::eDepth) {
		vk::BufferImageCopy region;
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D{ 0,0,0 };
		region.imageExtent = m_vkExtent;

		regions.push_back(region);
	}

	if (m_vkAspectFlags & vk::ImageAspectFlagBits::eStencil) {
		vk::BufferImageCopy region;
		region.bufferOffset = m_vkExtent.width * m_vkExtent.height * sizeof(float);
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eStencil;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D{ 0, 0 ,0 };
		region.imageExtent = m_vkExtent;

		regions.push_back(region);
	}

	commandBuffer->copyBufferToImage(*buffer, m_vkImage, vk::ImageLayout::eTransferDstOptimal, regions.size(), regions.data());

	//Transition to eGeneral, as that is our "default" layout
	transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral>(commandBuffer);
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

	auto bufferSize = m_size;
	//if image came from swapchain (i.e. m_image == 0)
	if (bufferSize == 0) {
		auto memoryRequirements = m_vkDevice->getImageMemoryRequirements(m_vkImage);
		bufferSize = memoryRequirements.size;
	}

	vk::BufferCreateInfo bufferInfo = {};
	bufferInfo.setSize(bufferSize)
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
	region.imageSubresource.aspectMask = m_vkAspectFlags; //TODO: subresource.aspectMask can only have 1 bit set. Handle case where *this is a depth/stencil buffer. -AM
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = vk::Offset3D{ 0,0,0 };
	region.imageExtent = m_vkExtent;

	commandBuffer->copyImageToBuffer(m_vkImage, vk::ImageLayout::eTransferSrcOptimal, *buffer, { region });
	transition<vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral>(commandBuffer);

	commandBuffer->end();

	//execute everything
	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*commandBuffer);

	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
	m_device.m_vkInternalQueue.waitIdle();

	//cleanup
	commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	std::vector<char> result(bufferSize);
	auto mappedMemory = m_vkDevice->mapMemory(*memory, 0, VK_WHOLE_SIZE);
	memcpy(result.data(), mappedMemory, bufferSize);
	m_vkDevice->unmapMemory(*memory);
	return result;
}

uint32_t ImageResource::getWidth() const
{
	return m_vkExtent.width;
}

uint32_t ImageResource::getHeight() const
{
	return m_vkExtent.height;
}

Format ImageResource::getFormat() const
{
	return  from_vulkan_format(m_format);
}


ImageResource ImageResource::createDepthResource(
	const Device& device, 
	vk::Extent3D extent,
	const std::vector<vk::Format>& formatCandidates)
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
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst |vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled)
		.setSamples(vk::SampleCountFlagBits::e1));

	auto memoryRequirements = device.m_vkDevice->getImageMemoryRequirements(image);

	auto aspectFlags = vk::ImageAspectFlags();

	switch (format) {
		case vk::Format::eS8Uint:
			aspectFlags = vk::ImageAspectFlagBits::eStencil;
			break;
		case vk::Format::eD32SfloatS8Uint:
		case vk::Format::eD24UnormS8Uint:
			aspectFlags = vk::ImageAspectFlagBits::eDepth |  vk::ImageAspectFlagBits::eStencil;
			break;
		case vk::Format::eD32Sfloat:
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
			break;
		default:
			PAPAGO_ERROR("Unimplemented depth/stencil format!");
			break;
	}

	return ImageResource(
		image,
		device,
		aspectFlags,
		format,
		extent,
		memoryRequirements);
}

ImageResource ImageResource::createColorResource(
	vk::Image image, 
	const Device& device, 
	vk::Format format,
	vk::Extent3D extent)
{
	return ImageResource(image, device, format, extent);
}

// Allocates memory to the image and creates an image view to the provided image
ImageResource::ImageResource(
	vk::Image& image,
	const Device& device,
	vk::ImageAspectFlags aspectFlags,
	vk::Format format,
	vk::Extent3D extent,
	vk::MemoryRequirements memoryRequirements)
	: Resource(device.m_vkPhysicalDevice, device.m_vkDevice, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible, memoryRequirements)
	, m_vkImage(image)
	, m_format(format)
	, m_vkExtent(extent)
	, m_device(device)
	, m_vkAspectFlags(aspectFlags)
{
	m_vkDevice->bindImageMemory(m_vkImage, *m_vkMemory, 0);

	createImageView(m_vkDevice, aspectFlags);

	vk::CommandBufferBeginInfo info = {};
	info.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	m_device.m_internalCommandBuffer->begin(info);

	if (aspectFlags & vk::ImageAspectFlagBits::eDepth) {
		transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral>(m_device.m_internalCommandBuffer);
	}
	else if (aspectFlags & vk::ImageAspectFlagBits::eColor) {
		transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral>(m_device.m_internalCommandBuffer);
	}
	
	m_device.m_internalCommandBuffer->end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*m_device.m_internalCommandBuffer);
	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
	m_device.m_vkInternalQueue.waitIdle();

	m_device.m_internalCommandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	
	auto clearVector = std::vector<char>(memoryRequirements.size);
	auto clearFloats = std::vector<float>(m_vkExtent.width * m_vkExtent.height, 1.0f);
	auto clearStencil = std::vector<unsigned char>(memoryRequirements.size, 1);

	if (aspectFlags & vk::ImageAspectFlagBits::eDepth) {
		memcpy(clearVector.data(), clearFloats.data(), clearFloats.size() * sizeof(float));
		
		memcpy(clearVector.data() + (clearFloats.size() * sizeof(float)), clearStencil.data(), clearVector.size() - clearFloats.size() * sizeof(float) - 1);
	}
	
	upload(clearVector);
}

// Does NOT allocate memory, this is assumed to already be allocated; but does create a VkImageView.
ImageResource::ImageResource(vk::Image& image, const Device& device, vk::Format format, vk::Extent3D extent)
	: Resource(device.m_vkDevice)
	, m_vkImage(std::move(image))
	, m_format(format)
	, m_vkExtent(extent)
	, m_device(device)
	, m_vkAspectFlags(vk::ImageAspectFlagBits::eColor)
{
	createImageView(m_vkDevice);

	vk::CommandBufferBeginInfo info = {};
	info.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	m_device.m_internalCommandBuffer->begin(info);
	
	transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral>(m_device.m_internalCommandBuffer);

	m_device.m_internalCommandBuffer->end();
	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*m_device.m_internalCommandBuffer);
	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
	m_device.m_vkInternalQueue.waitIdle();

	m_device.m_internalCommandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	auto clearVector = std::vector<char>(m_vkExtent.width * m_vkExtent.height * m_vkExtent.depth * sizeOfFormat(m_format));
	upload(clearVector);
}

inline bool ImageResource::inUse()
{
	return m_vkFence
		&& m_vkDevice->getFenceStatus(*m_vkFence) == vk::Result::eNotReady;
}

vk::Format ImageResource::findSupportedFormat(
	const vk::PhysicalDevice & physicalDevice, 
	const std::vector<vk::Format>& candidateFormats, 
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
