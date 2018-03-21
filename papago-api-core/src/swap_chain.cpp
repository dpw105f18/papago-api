#include "standard_header.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"

SwapChain::SwapChain(vk::Device &device, vk::SwapchainKHR &swapChain, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D extent) 
	: m_VkSwapChain(swapChain), m_colorResources(colorResources), m_depthResources(depthResources)
{

	// the vk::RenderPass set on the Framebuffers is a guideline for what RenderPass' are compatible with them
	// this _may_ be error-prone...
	auto renderPass = createDummyRenderPass(device);

	for (auto i = 0; i < colorResources.size(); ++i) {
		std::vector<vk::ImageView> attachments = {
			colorResources[i].m_VkImageView,
			depthResources[i].m_VkImageView
		};

		vk::FramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.setRenderPass(renderPass)
			.setAttachmentCount(attachments.size())
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);

		m_Framebuffers.emplace_back(device.createFramebuffer(framebufferCreateInfo));
	}
}

vk::RenderPass SwapChain::createDummyRenderPass(const vk::Device& device)
{
	vk::AttachmentDescription colorDesc = {};
	colorDesc.setFormat(m_colorResources[0].m_Format)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentDescription depthDesc = {};
	depthDesc.setFormat(m_depthResources[0].m_Format)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference colorRef(0);
	vk::AttachmentReference depthRef(1);

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorRef)
		.setPDepthStencilAttachment(&depthRef);

	vk::SubpassDependency subpassDependency(VK_SUBPASS_EXTERNAL); //TODO: find the "enum" in vulkan.hpp
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

	return device.createRenderPass(renderPassCreateInfo);
}
