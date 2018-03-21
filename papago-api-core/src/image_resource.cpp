#include "standard_header.hpp"
#include "image_resource.hpp"

void ImageResource::upload(std::vector<char> data)
{
}

void ImageResource::destroy()
{
}

void ImageResource::download()
{
}

ImageResource ImageResource::createDepthResource(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, size_t width, size_t height, const std::vector<Format>& formatCandidates)
{
	auto format = ImageResource::findSupportedFormat(physicalDevice, formatCandidates, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto extent = vk::Extent3D().setWidth(width)
		.setHeight(height)
		.setDepth(1);

	vk::ImageCreateInfo createInfo = {};
	createInfo.setImageType(vk::ImageType::e2D)
		.setExtent(extent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setSamples(vk::SampleCountFlagBits::e1);


	return ImageResource(createInfo, physicalDevice, device);
}

ImageResource ImageResource::createColorResource(vk::Image image, const vk::Device &device, Format format)
{
	return ImageResource(image, device, format);
}

ImageResource::ImageResource(vk::ImageCreateInfo imageCreateInfo, const vk::PhysicalDevice& physicalDevice, const vk::Device& device) 
	: m_VkCreateInfo(imageCreateInfo), m_Format(imageCreateInfo.format)
{
	m_VkImage = device.createImage(imageCreateInfo);

	vk::MemoryRequirements memoryRequirements = device.getImageMemoryRequirements(m_VkImage);

	uint32_t memoryType;

	auto memoryProperties = physicalDevice.getMemoryProperties();
	for (auto i = 0; i < memoryProperties.memoryTypeCount; ++i) {
		if (memoryRequirements.memoryTypeBits & (1 << i) &&
			memoryProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) { //<-- TODO: make memPropFlagBit setable
			memoryType = i;
			break;
		}
	}

	vk::MemoryAllocateInfo allocateInfo = {};
	allocateInfo.setAllocationSize(memoryRequirements.size)
		.setMemoryTypeIndex(memoryType);

	m_VkMemory = device.allocateMemory(allocateInfo);

	device.bindImageMemory(m_VkImage, m_VkMemory, 0);

	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setLevelCount(1)
		.setLayerCount(1);



	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.setImage(m_VkImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_Format)
		.setSubresourceRange(subresourceRange);

	m_VkImageView = device.createImageView(viewCreateInfo);

	//TODO: transition image (via command buffers)

}

ImageResource::ImageResource(vk::Image image, const vk::Device &device, Format format): m_VkImage(image), m_Format(format)
{
	setImageView(device);
}

Format ImageResource::findSupportedFormat(const vk::PhysicalDevice & physicalDevice, const std::vector<Format>& candidateFormats, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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

void ImageResource::setImageView(const vk::Device &device)
{
	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setLevelCount(1)
		.setLayerCount(1);


	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.setImage(m_VkImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_Format)
		.setSubresourceRange(subresourceRange);

	m_VkImageView = device.createImageView(viewCreateInfo);
}
