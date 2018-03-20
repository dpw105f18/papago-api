#include "standard_header.hpp"
#include "swap_chain.hpp"

SwapChain::SwapChain(vk::Device& device, vk::SwapchainKHR& swapChain, Format& format):m_VkSwapChain(swapChain), m_Format(format)
{
	m_Images = device.getSwapchainImagesKHR(m_VkSwapChain);
	m_ImageViews.resize(m_Images.size());

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
}
