#include "standard_header.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"

SwapChain::operator vk::SwapchainKHR&()
{
	return *m_vkSwapChain;
}

uint32_t SwapChain::getWidth() const
{
	return m_vkExtent.width;
}

uint32_t SwapChain::getHeight() const
{
	return m_vkExtent.height;
}

Format SwapChain::getFormat() const
{
	//TODO: make sure swapchain ALWAYS have at least one colorResource! -AM
	return m_colorResources[0].getFormat();
}

SwapChain::SwapChain(
	const Device&				device, 
	vk::UniqueSwapchainKHR&		swapChain, 
	std::vector<ImageResource>& colorResources, 
	std::vector<ImageResource>& depthResources, 
	vk::Extent2D				extent) 
	: m_vkSwapChain(std::move(swapChain))
	, m_colorResources(std::move(colorResources))
	, m_depthResources(std::move(depthResources))
	, m_vkExtent(extent)
{
	// the vk::RenderPass set on the Framebuffers is a guideline for what RenderPass' are compatible with them
	// this _may_ be error-prone...
	m_vkRenderPass = device.createVkRenderpass(m_colorResources[0].m_format, m_depthResources[0].m_format);
	auto swapChainSize = m_colorResources.size();

	for (auto i = 0; i < swapChainSize; ++i) {
		std::vector<vk::ImageView> attachments = {
			*m_colorResources[i].m_vkImageView,
			*m_depthResources[i].m_vkImageView
		};

		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.setRenderPass(*m_vkRenderPass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);

		m_vkFramebuffers.emplace_back(device.getVkDevice()->createFramebufferUnique(framebufferCreateInfo));
		m_vkFences.emplace_back(device.getVkDevice()->createFenceUnique(vk::FenceCreateInfo{}));
	}
}

SwapChain::SwapChain(const Device &device, vk::UniqueSwapchainKHR &swapChain, std::vector<ImageResource>& colorResources, vk::Extent2D extent)
	: m_vkSwapChain(std::move(swapChain))
	, m_colorResources(std::move(colorResources))
	, m_vkExtent(extent)
{
	m_vkRenderPass = device.createVkRenderpass(m_colorResources[0].m_format);
	auto swapChainSize = m_colorResources.size();

	for (auto i = 0; i < swapChainSize; ++i) {
		std::vector<vk::ImageView> attachments = {
			*m_colorResources[i].m_vkImageView,
		};

		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.setRenderPass(*m_vkRenderPass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);

		m_vkFramebuffers.emplace_back(device.getVkDevice()->createFramebufferUnique(framebufferCreateInfo));
		m_vkFences.emplace_back(device.getVkDevice()->createFenceUnique(vk::FenceCreateInfo{}));
	}
}
