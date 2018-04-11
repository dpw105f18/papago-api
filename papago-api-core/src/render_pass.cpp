#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "vertex.hpp"

RenderPass::operator vk::RenderPass&()
{
	return *m_vkRenderPass;
}

RenderPass::RenderPass(
	const vk::UniqueDevice& device,
	const VertexShader& vertexShader,
	const FragmentShader& fragmentShader,
	const vk::Extent2D& extent,
	Format format)
	: m_vertexShader(vertexShader), m_fragmentShader(fragmentShader)
{
	vk::PipelineShaderStageCreateInfo shaderStages[] = { 
		m_vertexShader.m_vkStageCreateInfo, 
		m_fragmentShader.m_vkStageCreateInfo 
	};

	//TODO: Find a better generic way to set attribute and binding descriptions up
	auto attributeDescription = Vertex::getAttributeDescriptions();
	auto bindingDescription = Vertex::getBindingDescription();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&bindingDescription)
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescription.size()))
		.setPVertexAttributeDescriptions(attributeDescription.data()); 

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
		.setFrontFace(vk::FrontFace::eClockwise)
		.setRasterizerDiscardEnable(VK_FALSE);

	vk::PipelineColorBlendAttachmentState colorBlendAttatchment;
	colorBlendAttatchment.setColorWriteMask(
		vk::ColorComponentFlagBits::eR
		| vk::ColorComponentFlagBits::eG
		| vk::ColorComponentFlagBits::eB
		| vk::ColorComponentFlagBits::eA)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setBlendEnable(1);

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

	m_vkRenderPass =  createDummyRenderpass(device, format);

	vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.0f);

	vk::PipelineDepthStencilStateCreateInfo depthCreateInfo = {};
	depthCreateInfo.setDepthTestEnable(true)
		.setDepthWriteEnable(true)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setMaxDepthBounds(1.0f);

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
		.setLayout(m_vkPipelineLayout.get())
		.setPMultisampleState(&multisampleCreateInfo)
		.setPDepthStencilState(&depthCreateInfo);

	m_vkGraphicsPipeline = device->createGraphicsPipelineUnique(vk::PipelineCache(), pipelineCreateInfo);
}

//TODO: Make the creation of vkRenderPass more flexible. This dummy only makes one color-attachment and a depth-attachment
vk::UniqueRenderPass RenderPass::createDummyRenderpass(const vk::UniqueDevice& device, Format format) {

	vk::AttachmentDescription colorAttachment;
	colorAttachment.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference colorAttatchmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentDescription depthAttachment;
	depthAttachment.setFormat(vk::Format::eD32Sfloat)
		.setLoadOp(vk::AttachmentLoadOp::eClear) // Clear buffer data at load
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	std::vector<vk::AttachmentDescription> attachments = { colorAttachment, depthAttachment };
	std::vector<vk::AttachmentReference> attachmentReferences = { colorAttatchmentRef, depthAttachmentRef };

	vk::SubpassDescription subpass = {};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttatchmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL);
	dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags())
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);


	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	return device->createRenderPassUnique(renderPassInfo);
}
