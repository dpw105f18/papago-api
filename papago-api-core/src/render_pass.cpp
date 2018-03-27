#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"

RenderPass::RenderPass(
	const vk::UniqueDevice& device,
	VertexShader& vertexShader,
	FragmentShader& fragmentShader,
	const vk::Extent2D& extent,
	Format format)
	: m_vertexShader(vertexShader), m_fragmentShader(fragmentShader)
{
	vk::PipelineShaderStageCreateInfo shaderStages[] = { m_vertexShader.m_vkStageCreateInfo, m_fragmentShader.m_vkStageCreateInfo };

	//TODO: use our vertex buffer
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.setVertexBindingDescriptionCount(0)
		.setPVertexBindingDescriptions(nullptr)
		.setVertexAttributeDescriptionCount(0)
		.setPVertexAttributeDescriptions(nullptr);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);	//<-- TODO: make setable

	auto viewport = vk::Viewport(0.0f, 0.0f, extent.width, extent.height, 0.0f, 1.0f);
	auto scissor = vk::Rect2D({ 0, 0 }, extent);

	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.setViewportCount(1)
		.setPViewports(&viewport)
		.setScissorCount(1)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer;
	rasterizer.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setRasterizerDiscardEnable(VK_TRUE);

	vk::PipelineColorBlendAttachmentState colorBlendAttatchment;
	colorBlendAttatchment.setColorWriteMask(
		vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB
		| vk::ColorComponentFlagBits::eA);

	vk::PipelineColorBlendStateCreateInfo colorBlending;
	colorBlending.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttatchment);

	//TODO: expand once we have vkDescriptorSets (from Parser)
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setSetLayoutCount(0)
		.setPSetLayouts(nullptr)
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(nullptr);

	m_vkPipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutInfo);

	m_vkRenderPass =  std::move(createDummyRenderpass(device, format));

	// Not expecting vertex buffer or depth test 
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.setStageCount(2)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPColorBlendState(&colorBlending)
		.setRenderPass(m_vkRenderPass.get())
		.setLayout(m_vkPipelineLayout.get());

	m_vkGraphicsPipeline = std::move(device->createGraphicsPipelineUnique(vk::PipelineCache(), pipelineCreateInfo));
}

//TODO: Make the creation of vkRenderPass more flexible. This dummy only makes one color-attachment (no depth)
vk::UniqueRenderPass RenderPass::createDummyRenderpass(const vk::UniqueDevice& device, Format format) {

	vk::AttachmentDescription colorAttatchment;
	colorAttatchment.setFormat(format)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference colorAttatchmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttatchmentRef);

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL);
	dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags())
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(1)
		.setPAttachments(&colorAttatchment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	return std::move(device->createRenderPassUnique(renderPassInfo));
}
