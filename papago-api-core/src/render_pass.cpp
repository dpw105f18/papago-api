#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "shader_program.h"
#include "image_resource.hpp"
#include "sampler.hpp"

RenderPass::operator vk::RenderPass&()
{
	return *m_vkRenderPass;
}

RenderPass::RenderPass(
	const vk::UniqueDevice& device,
	vk::UniqueRenderPass& vkRenderPass,
	const ShaderProgram& program,
	const vk::Extent2D& extent,
	DepthStencilFlags depthStencilFlags)
	: m_shaderProgram(program)
	, m_vkDevice(device)
	, m_vkRenderPass(std::move(vkRenderPass))
	, m_depthStencilFlags(depthStencilFlags)
{
	setupDescriptorSet(device, program.m_vertexShader, program.m_fragmentShader);

	vk::PipelineShaderStageCreateInfo shaderStages[] = { 
		program.m_vkVertexStageCreateInfo,
		program.m_vkFragmentStageCreateInfo
	};
	
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	//do the shader require a vertex buffer?
	auto attributeDescription = getAttributeDescriptions();
	vk::VertexInputBindingDescription bindingDescription;

	if (!m_shaderProgram.m_vertexShader.m_input.empty()) {

		bindingDescription = getBindingDescription();

		//TODO: how to handle vertex buffer existence and count? -AM
		vertexInputInfo.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&bindingDescription)
			.setVertexAttributeDescriptionCount(attributeDescription.size())
			.setPVertexAttributeDescriptions(attributeDescription.data()); 
	}
	else {
		vertexInputInfo.setVertexBindingDescriptionCount(0)
			.setPVertexBindingDescriptions(nullptr)
			.setVertexAttributeDescriptionCount(0)
			.setPVertexAttributeDescriptions(nullptr);
	}

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
		.setCullMode(vk::CullModeFlagBits::eFront)
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
	if (m_vkDescriptorSetLayout) {
		pipelineLayoutInfo.setSetLayoutCount(1)
			.setPSetLayouts(&m_vkDescriptorSetLayout.get());
	}

	m_vkPipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutInfo);

	vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.0f);

	
	vk::PipelineDepthStencilStateCreateInfo* depthCreateInfo = nullptr;
	vk::PipelineDepthStencilStateCreateInfo emptyDepthCreateInfo = {};
	if (depthStencilFlags != DepthStencilFlags::eNone) {
		depthCreateInfo = &emptyDepthCreateInfo;

		if ((depthStencilFlags & DepthStencilFlags::eDepth) != DepthStencilFlags::eNone) {
			depthCreateInfo->setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setMaxDepthBounds(1.0f);
		}

		if ((depthStencilFlags & DepthStencilFlags::eStencil) != DepthStencilFlags::eNone) {
			auto back = vk::StencilOpState()
				.setFailOp(vk::StencilOp::eKeep)
				.setPassOp(vk::StencilOp::eKeep)
				.setCompareOp(vk::CompareOp::eAlways)
				.setCompareMask(0xff)
				.setWriteMask(0xff);

			auto front = vk::StencilOpState()
				.setFailOp(vk::StencilOp::eKeep)
				.setPassOp(vk::StencilOp::eKeep)
				.setCompareOp(vk::CompareOp::eAlways)
				.setCompareMask(0xff)
				.setWriteMask(0xff);

			depthCreateInfo->setStencilTestEnable(true)
				.setBack(back)
				.setFront(front);
		}
	}

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
		.setPDepthStencilState(depthCreateInfo);

	m_vkGraphicsPipeline = device->createGraphicsPipelineUnique(vk::PipelineCache(), pipelineCreateInfo);
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

	if (!vkBindings.empty()) {
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
}

vk::VertexInputBindingDescription RenderPass::getBindingDescription()
{
	auto& inputs = m_shaderProgram.m_vertexShader.m_input;
	auto& lastInput = inputs.back();
	vk::VertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = lastInput.offset + lastInput.getFormatSize();
	bindingDescription.inputRate = vk::VertexInputRate::eVertex; //VK_VERTEX_INPUT_RATE_VERTEX

	return bindingDescription;
}

std::vector<vk::VertexInputAttributeDescription> RenderPass::getAttributeDescriptions()
{
	auto inputCount = m_shaderProgram.m_vertexShader.m_input.size();
	auto attributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(inputCount);

	for (auto i = 0; i < inputCount; ++i) {
		auto input = m_shaderProgram.m_vertexShader.m_input[i];
		attributeDescriptions[i].setBinding(0)
			.setLocation(i)
			.setFormat(input.format)
			.setOffset(input.offset);
	}

	return attributeDescriptions;
}
