#include "standard_header.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"

SwapChain::operator vk::SwapchainKHR&()
{
	return *m_vkSwapChain;
}

SwapChain::SwapChain(
	vk::UniqueDevice&			device, 
	vk::UniqueSwapchainKHR&		swapChain, 
	std::vector<ImageResource>& colorResources, 
	std::vector<ImageResource>& depthResources, 
	vk::Extent2D				extent) 
	: m_vkSwapChain(std::move(swapChain))
	, m_colorResources(std::move(colorResources))
	, m_depthResources(std::move(depthResources))
{
	// the vk::RenderPass set on the Framebuffers is a guideline for what RenderPass' are compatible with them
	// this _may_ be error-prone...
	m_vkRenderPass = createDummyRenderPass(device);

	for (auto i = 0; i < m_colorResources.size(); ++i) {
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

		m_framebuffers.emplace_back(device->createFramebufferUnique(framebufferCreateInfo));
	}
}

vk::UniqueRenderPass SwapChain::createDummyRenderPass(const vk::UniqueDevice& device)
{
	vk::AttachmentDescription colorDesc = {};
	colorDesc.setFormat(m_colorResources[0].m_format)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthDesc = {};
	depthDesc.setFormat(m_depthResources[0].m_format)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorRef)
		.setPDepthStencilAttachment(&depthRef);

	vk::SubpassDependency subpassDependency(VK_SUBPASS_EXTERNAL);
	subpassDependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags())
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::vector<vk::AttachmentDescription> attachments = { colorDesc, depthDesc };

	vk::RenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&subpassDependency);

	return device->createRenderPassUnique(renderPassCreateInfo);
}
