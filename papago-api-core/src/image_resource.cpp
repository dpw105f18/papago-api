#include "standard_header.hpp"
#include "image_resource.hpp"

ImageResource::ImageResource(ImageResource&& other) noexcept
	: Resource(std::move(other))
	, m_vkImage(std::move(other.m_vkImage))
	, m_vkImageView(std::move(other.m_vkImageView))
	, m_format(other.m_format)
{
	other.m_format = Format();
}

ImageResource::~ImageResource()
{
	// HACK: If size is zero then memory was externally allocated
	if (getSize()) {
		m_vkDevice->destroyImage(m_vkImage);
	}
}

void ImageResource::destroy()
{
}

ImageResource ImageResource::createDepthResource(
	const vk::PhysicalDevice &physicalDevice, 
	const vk::UniqueDevice &device, 
	size_t width, size_t height, 
	const std::vector<Format>& formatCandidates)
{
	auto format = findSupportedFormat(
		physicalDevice, 
		formatCandidates, 
		vk::ImageTiling::eOptimal, 
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto extent = vk::Extent3D()
		.setWidth(width)
		.setHeight(height)
		.setDepth(1);

	auto image = device->createImage(vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setExtent(extent)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setSamples(vk::SampleCountFlagBits::e1)
	);
	auto memoryRequirements = device->getImageMemoryRequirements(image);

	return ImageResource(
		image, 
		physicalDevice, 
		device, 
		vk::ImageAspectFlagBits::eDepth, 
		format, 
		memoryRequirements);
}

ImageResource ImageResource::createColorResource(
	vk::Image image, 
	const vk::UniqueDevice &device, 
	Format format)
{
	return ImageResource(image, device, format);
}

// Allocates memory to the image and creates an image view to the provided image
ImageResource::ImageResource(
	vk::Image& image,
	const vk::PhysicalDevice& physicalDevice, 
	const vk::UniqueDevice& device, 
	vk::ImageAspectFlags aspectFlags, 
	Format format, 
	vk::MemoryRequirements memoryRequirements) 
		: Resource(physicalDevice, device, vk::MemoryPropertyFlagBits::eDeviceLocal, memoryRequirements)
		, m_vkImage(image)
		, m_format(format)
{
	device->bindImageMemory(m_vkImage, *m_vkMemory, 0);

	createImageView(device, aspectFlags);

	//TODO: transition image (via command buffers)
}

// Does NOT allocate memory, this is assumed to already be allocated; but does create a VkImageView.
ImageResource::ImageResource(vk::Image& image, const vk::UniqueDevice &device, Format format)
	: Resource(device)
	, m_vkImage(std::move(image))
	, m_format(format)
{
	createImageView(device);
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
