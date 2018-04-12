#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "vertex.hpp"
#include "shader_program.h"
#include "image_resource.hpp"
#include "sampler.hpp"

RenderPass::operator vk::RenderPass&()
{
	return *m_vkRenderPass;
}



RenderPass::RenderPass(
	const vk::UniqueDevice& device,
	const ShaderProgram& program,
	const vk::Extent2D& extent,
	Format format)
		:	m_shaderProgram(program), m_vkDevice(device)
{
	setupDescriptorSet(device, program.m_vertexShader, program.m_fragmentShader);


	vk::PipelineShaderStageCreateInfo shaderStages[] = { 
		program.m_vkVertexStageCreateInfo,
		program.m_vkFragmentStageCreateInfo
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

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setSetLayoutCount(1)
		.setPSetLayouts(&m_vkDescriptorSetLayout.get());

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

void RenderPass::setupDescriptorSet(const vk::UniqueDevice &device, const VertexShader& vertexShader, const FragmentShader& fragmentShader)
{
	//Descriptor Set Layout
	std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
	std::map<uint32_t, size_t> bindingMap;

	auto vertexBindings = vertexShader.getBindings();
	for (size_t i = 0; vertexBindings.size(); ++i) {
		auto& vertexBinding = vertexBindings[i];

		vk::DescriptorSetLayoutBinding binding = {};
		binding.setBinding(vertexBinding.binding)
			.setDescriptorCount(1) //TODO: can we assume 1 Descriptor per binding? -AM
			.setDescriptorType(vertexBinding.type)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vkBindings.emplace_back(binding);

		bindingMap.insert(std::pair<uint32_t, size_t>{vertexBinding.binding, i});
	}

	auto fragmentBindings = fragmentShader.getBindings();
	for (size_t i = 0; i < fragmentBindings.size(); ++i) {
		//if the binding was used by VertexShader:
		if (bindingMap.size() > 0 && bindingMap.find(i) == bindingMap.end()) {
			vkBindings[i].stageFlags |= vk::ShaderStageFlagBits::eFragment;
		}
		else {
			auto& fragmentBinding = fragmentBindings[i];
			vk::DescriptorSetLayoutBinding binding = {};
			binding.setBinding(fragmentBinding.binding)
				.setDescriptorCount(1) //TODO: can we assume 1 Descriptor per binding? -AM
				.setDescriptorType(fragmentBinding.type)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vkBindings.emplace_back(binding);
		}
	}

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.setBindingCount(vkBindings.size())
		.setPBindings(vkBindings.data());

	m_vkDescriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutCreateInfo);


	//Descriptor Pool:
	auto poolSizes = std::vector<vk::DescriptorPoolSize>(vkBindings.size());
	for (auto i = 0; i < vkBindings.size(); ++i) {
		auto& vkBinding = vkBindings[i];
		poolSizes[i].setDescriptorCount(1)
			.setType(vkBinding.descriptorType);
	}

	vk::DescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.setPoolSizeCount(poolSizes.size())
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(1)	//TODO: keep this default value? -AM.
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	m_vkDescriptorPool = device->createDescriptorPoolUnique(poolCreateInfo);


	//Descriptor Set:
	vk::DescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.setDescriptorPool(*m_vkDescriptorPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&m_vkDescriptorSetLayout.get());

	m_vkDescriptorSet = std::move(device->allocateDescriptorSetsUnique(allocateInfo)[0]);	//TODO: do we always want exactly one descriptor set? -AM
}
