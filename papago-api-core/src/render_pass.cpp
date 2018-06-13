#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "shader_program.hpp"
#include "buffer_resource.hpp"

RenderPass::operator vk::RenderPass&()
{
	return *m_vkRenderPass;
}

RenderPass::RenderPass(
	const vk::UniqueDevice& device,
	vk::UniqueRenderPass& vkRenderPass,
	ShaderProgram& program,
	const vk::Extent2D& extent,
	DepthStencilFlags depthStencilFlags)
	: m_shaderProgram(program)
	, m_vkDevice(device)
	, m_vkRenderPass(std::move(vkRenderPass))
	, m_depthStencilFlags(depthStencilFlags)
	, m_vkExtent(extent)
{
	auto& vertBindings = m_shaderProgram.m_vertexShader.m_bindings;
	for (auto& vb : vertBindings) {
		auto name = vb.first;
		auto binding = vb.second.binding;
		m_namedBindings[name] = binding;
	}
	
	auto& fragBindings = m_shaderProgram.m_fragmentShader.m_bindings;
	for (auto& vb : fragBindings) {
		auto name = vb.first;
		auto binding = vb.second.binding;
		m_namedBindings[name] = binding;
	}

	if (program.getUniqueUniformBindings().empty()) {
		cacheNewPipeline(0);
	}

}

void RenderPass::setupDescriptorSetLayout(const vk::UniqueDevice &device, const VertexShader& vertexShader, const FragmentShader& fragmentShader, uint64_t bindingMask)
{
	//Descriptor Set Layout
	std::vector<vk::DescriptorSetLayoutBinding> vkBindings;
	std::map<uint32_t, size_t> bindingMap;

	auto vertexBindings = vertexShader.getBindings();
	for (size_t i = 0; i < vertexBindings.size(); ++i) {
		auto& vertexBinding = vertexBindings[i];
		
		vk::DescriptorType type = vertexBinding.type;
		auto bindingValue = vertexBinding.binding;
		if (type == vk::DescriptorType::eUniformBuffer) {
			type = (bindingMask & (1 << bindingValue)) ? vk::DescriptorType::eUniformBufferDynamic : vk::DescriptorType::eUniformBuffer;
		}

		vk::DescriptorSetLayoutBinding binding = {};
		binding.setBinding(bindingValue)
			.setDescriptorCount(1) //TODO: can we assume 1 Descriptor per binding? -AM
			.setDescriptorType(type)
			.setStageFlags(vk::ShaderStageFlagBits::eVertex);

		vkBindings.emplace_back(binding);

		bindingMap.insert(std::pair<uint32_t, size_t>{vertexBinding.binding, i});
	}

	auto fragmentBindings = fragmentShader.getBindings();
	for (size_t i = 0; i < fragmentBindings.size(); ++i) {
		auto& fragmentBinding = fragmentBindings[i];
		//if the binding was used by VertexShader:
		if (!bindingMap.empty() && bindingMap.find(fragmentBinding.binding) != bindingMap.end()) {
			auto vkBindingIndex = bindingMap[fragmentBinding.binding];
			vkBindings[vkBindingIndex].stageFlags |= vk::ShaderStageFlagBits::eFragment;
		}
		else {
			vk::DescriptorType type = fragmentBinding.type;
			auto bindingValue = fragmentBinding.binding;
			if (type == vk::DescriptorType::eUniformBuffer) {
				type = (bindingMask & (1 << bindingValue)) ? vk::DescriptorType::eUniformBufferDynamic : vk::DescriptorType::eUniformBuffer;
			}

			vk::DescriptorSetLayoutBinding binding = {};
			binding.setBinding(bindingValue)
				.setDescriptorCount(1) //TODO: can we assume 1 Descriptor per binding? -AM
				.setDescriptorType(type)
				.setStageFlags(vk::ShaderStageFlagBits::eFragment);

			vkBindings.emplace_back(binding);
			bindingMap.insert(std::pair<uint32_t, size_t>{fragmentBinding.binding, i});
		}
	}

	if (!vkBindings.empty()) {
		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
		layoutCreateInfo.setBindingCount(vkBindings.size())
			.setPBindings(vkBindings.data());

		m_vkDescriptorSetLayouts[bindingMask] = std::move(device->createDescriptorSetLayoutUnique(layoutCreateInfo));
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

vk::UniquePipeline & RenderPass::getPipeline(uint64_t mask)
{
	std::map<uint64_t, vk::UniquePipeline>::iterator it = m_vkGraphicsPipelines.find(mask);
	if (it == m_vkGraphicsPipelines.end()) {
		std::stringstream ss;
		ss << "Pipeline not found for mask " << mask << std::endl;
		PAPAGO_ERROR(ss.str());
	}
	else {
		return m_vkGraphicsPipelines[mask];
	}
}

vk::UniquePipelineLayout & RenderPass::getPipelineLayout(uint64_t mask)
{
	if (m_vkPipelineLayouts.find(mask) == m_vkPipelineLayouts.end()) {
		std::stringstream ss;
		ss << "PipelineLayout not found for mask " << mask << std::endl;
		PAPAGO_ERROR(ss.str());
	}
	else {
		return m_vkPipelineLayouts[mask];
	}
}

vk::UniqueDescriptorSetLayout & RenderPass::getDescriptorSetLayout(uint64_t mask)
{
	if (m_vkDescriptorSetLayouts.find(mask) == m_vkDescriptorSetLayouts.end()) {
		std::stringstream ss;
		ss << "DescriptorSetLayout not found for mask " << mask << std::endl;
		PAPAGO_ERROR(ss.str());
	}
	else {
		return m_vkDescriptorSetLayouts[mask];
	}
}

void RenderPass::cacheNewPipeline(uint64_t bindingMask)
{
	auto bindings = m_shaderProgram.getUniqueUniformBindings();

	setupDescriptorSetLayout(m_vkDevice, m_shaderProgram.m_vertexShader, m_shaderProgram.m_fragmentShader, bindingMask);

	vk::PipelineShaderStageCreateInfo shaderStages[] = {
		m_shaderProgram.m_vkVertexStageCreateInfo,
		m_shaderProgram.m_vkFragmentStageCreateInfo
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

	auto viewport = vk::Viewport(0.0f, 0.0f, m_vkExtent.width, m_vkExtent.height, 0.0f, 1.0f);
	auto scissor = vk::Rect2D({ 0, 0 }, m_vkExtent);

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
	if (m_vkDescriptorSetLayouts[bindingMask]) {
		pipelineLayoutInfo.setSetLayoutCount(1)
			.setPSetLayouts(&m_vkDescriptorSetLayouts[bindingMask].get());
	}

	m_vkPipelineLayouts[bindingMask] = m_vkDevice->createPipelineLayoutUnique(pipelineLayoutInfo);

	vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo = {};
	multisampleCreateInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.0f);


	vk::PipelineDepthStencilStateCreateInfo* depthCreateInfo = nullptr;
	vk::PipelineDepthStencilStateCreateInfo emptyDepthCreateInfo = {};
	if (m_depthStencilFlags != DepthStencilFlags::eNone) {
		depthCreateInfo = &emptyDepthCreateInfo;

		if ((m_depthStencilFlags & DepthStencilFlags::eDepth) != DepthStencilFlags::eNone) {
			depthCreateInfo->setDepthTestEnable(true)
				.setDepthWriteEnable(true)
				.setDepthCompareOp(vk::CompareOp::eLess)
				.setMaxDepthBounds(1.0f);
		}

		if ((m_depthStencilFlags & DepthStencilFlags::eStencil) != DepthStencilFlags::eNone) {
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

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.setStageCount(2)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPColorBlendState(&colorBlending)
		.setRenderPass(m_vkRenderPass.get())
		.setLayout(m_vkPipelineLayouts[bindingMask].get())
		.setPMultisampleState(&multisampleCreateInfo)
		.setPDepthStencilState(depthCreateInfo);

	m_vkGraphicsPipelines[bindingMask] = m_vkDevice->createGraphicsPipelineUnique(vk::PipelineCache(), pipelineCreateInfo);
}

void RenderPass::createNewPipelineIfNone(uint64_t mask)
{
	if(m_vkDescriptorSetLayouts.find(mask) == m_vkDescriptorSetLayouts.end())
		cacheNewPipeline(mask);
}

long RenderPass::getBinding(const std::string& name)
{
	if (m_namedBindings.find(name) != m_namedBindings.end()) {
		return m_namedBindings[name];
	}
	else {
		PAPAGO_ERROR("Invalid uniform name!");
	}
	return -1;
};