#include "standard_header.hpp"
#include "swap_chain.hpp"

SwapChain::SwapChain(vk::Device& device, vk::SwapchainKHR& swapChain, Format format, vk::Extent2D extent) : m_VkSwapChain(swapChain), m_Format(format), m_VkExtent(extent)
{
	m_Images = device.getSwapchainImagesKHR(m_VkSwapChain);
	auto framebufferCount = m_Images.size();
	m_ImageViews.resize(framebufferCount);

	for (auto i = 0; i < m_Images.size(); ++i) {
		vk::ImageViewCreateInfo view_info = {};
		view_info.image = m_Images[i];
		view_info.viewType = vk::ImageViewType::e2D;
		view_info.format = m_Format;
		view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		vk::ImageView image_view = device.createImageView(view_info);
		m_ImageViews[i] = image_view;
	}

	m_Framebuffers.resize(framebufferCount);

	for (auto i = 0; i < framebufferCount; ++i) {
		//vk::Image depthbuffer

		//TODO: PICK UP HERE! [Frambuffer story]
	}
}

vk::Image SwapChain::createDepthImage(const vk::PhysicalDevice& physicalDevice, const vk::Device& device) const
{
	std::vector<Format> formatCandidates = {
		Format::eD32Sfloat, Format::eD32SfloatS8Uint, Format::eD24UnormS8Uint
	};

	auto format = findSupportedFormat(physicalDevice, formatCandidates, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	auto extent = vk::Extent3D().setWidth(m_VkExtent.width)
		.setHeight(m_VkExtent.height)
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

	vk::Image result = device.createImage(createInfo);
	

	return vk::Image();
}

Format SwapChain::findSupportedFormat(const vk::PhysicalDevice &physicalDevice, const std::vector<Format>& candidateFormats, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
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

