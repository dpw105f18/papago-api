#include "standard_header.hpp"
#include "image_resource.hpp"

void ImageResource::upload(const std::vector<char>& data)
{
}

void ImageResource::destroy()
{
}

std::vector<char> ImageResource::download()
{
	return {};
}

ImageResource ImageResource::createDepthResource(const vk::PhysicalDevice &physicalDevice, const vk::UniqueDevice &device, size_t width, size_t height, const std::vector<Format>& formatCandidates)
{
	auto format = findSupportedFormat(physicalDevice, formatCandidates, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

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


	return ImageResource(createInfo, physicalDevice, device, vk::ImageAspectFlagBits::eDepth);
}

ImageResource ImageResource::createColorResource(vk::Image image, const vk::UniqueDevice &device, Format format)
{
	return ImageResource(image, device, format);
}

ImageResource::ImageResource(vk::ImageCreateInfo imageCreateInfo, const vk::PhysicalDevice& physicalDevice, const vk::UniqueDevice& device, vk::ImageAspectFlags aspectFlags) 
	: m_vkCreateInfo(imageCreateInfo), m_format(imageCreateInfo.format)
{
	m_vkImage = device.get().createImage(imageCreateInfo);

	vk::MemoryRequirements memoryRequirements = device.get().getImageMemoryRequirements(m_vkImage);

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

	m_vkMemory = device.get().allocateMemory(allocateInfo);

	device.get().bindImageMemory(m_vkImage, m_vkMemory, 0);

	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.setAspectMask(aspectFlags)
		.setLevelCount(1)
		.setLayerCount(1);



	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.setImage(m_vkImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_format)
		.setSubresourceRange(subresourceRange);

	m_vkImageView = device->createImageView(viewCreateInfo);

	//TODO: transition image (via command buffers)

}

ImageResource::ImageResource(vk::Image image, const vk::UniqueDevice &device, Format format): m_vkImage(image), m_format(format)
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

void ImageResource::setImageView(const vk::UniqueDevice &device)
{
	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setLevelCount(1)
		.setLayerCount(1);


	vk::ImageViewCreateInfo viewCreateInfo;
	viewCreateInfo.setImage(m_vkImage)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_format)
		.setSubresourceRange(subresourceRange);

	m_vkImageView = device->createImageView(viewCreateInfo);
}
