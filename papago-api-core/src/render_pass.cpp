#include "standard_header.hpp"
#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "shader_program.hpp"
#include "sampler.hpp"
#include "buffer_resource.hpp"
#include "image_resource.hpp"

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
	, m_vkExtent(extent)
{

	cacheNewPipeline(0x00);
}

void RenderPass::setupDescriptorSet(const vk::UniqueDevice &device, const VertexShader& vertexShader, const FragmentShader& fragmentShader, uint64_t bindingMask)
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
		//if the binding was used by VertexShader:
		if (bindingMap.size() > 0 && bindingMap.find(i) == bindingMap.end()) {
			vkBindings[i].stageFlags |= vk::ShaderStageFlagBits::eFragment;
		}
		else {
			auto& fragmentBinding = fragmentBindings[i];
			vk::DescriptorType type = fragmentBinding.type;
			auto bindingValue = fragmentBinding.binding;
			if (type == vk::DescriptorType::eUniformBuffer) {
				type = (bindingMask & (1 << bindingValue)) ? vk::DescriptorType::eUniformBufferDynamic : vk::DescriptorType::eUniformBuffer;
			}

			vk::DescriptorSetLayoutBinding binding = {};
			binding.setBinding(bindingValue)
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

		m_vkDescriptorSetLayouts[bindingMask] = std::move(device->createDescriptorSetLayoutUnique(layoutCreateInfo));


		//Descriptor Pool:
		// TODO: Use only one descriptor pool - Brandborg
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

		m_vkDescriptorPools[bindingMask] = std::move(device->createDescriptorPoolUnique(poolCreateInfo));


		//Descriptor Set:
		vk::DescriptorSetAllocateInfo allocateInfo = {};
		allocateInfo.setDescriptorPool(*m_vkDescriptorPools[bindingMask])
			.setDescriptorSetCount(1)
			.setPSetLayouts(&m_vkDescriptorSetLayouts[bindingMask].get());

		m_vkDescriptorSets[bindingMask] = std::move(device->allocateDescriptorSetsUnique(allocateInfo)[0]);	//TODO: do we always want exactly one descriptor set? -AM
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

void RenderPass::cacheNewPipeline(uint64_t bindingMask)
{
	auto bindings = m_shaderProgram.getUniqueUniformBindings();

	setupDescriptorSet(m_vkDevice, m_shaderProgram.m_vertexShader, m_shaderProgram.m_fragmentShader, bindingMask);

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

void RenderPass::bindResource(const std::string & name, IBufferResource &buffer)
{
	auto& innerBuffer = dynamic_cast<BufferResource&>(buffer);

	auto binding = getBinding(name);

	auto oldMask = m_descriptorSetKeyMask;
	//update mask so this binding bit is set to 0:
	m_descriptorSetKeyMask &= (~0x00 & (0x0 << binding));

	//if we have not cached a descriptor set
	if (m_vkDescriptorSets.find(m_descriptorSetKeyMask) == m_vkDescriptorSets.end())
	{
		cacheNewPipeline(m_descriptorSetKeyMask);
	}

	auto& descriptorSet = m_vkDescriptorSets[m_descriptorSetKeyMask];

	vk::DescriptorBufferInfo info = innerBuffer.m_vkInfo;

	std::vector<vk::CopyDescriptorSet> descriptorSetCopys;


	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&info);

	//only copy "old" binding information (for other bindings than this) if we have just created a new descriptor set:
	if (oldMask != m_descriptorSetKeyMask) {
		for (auto& bindingAligment : m_bindingAlignment)
		{
			auto otherBinding = bindingAligment.first;

			if (otherBinding != binding) {
				auto copyDescriptorSet = vk::CopyDescriptorSet()
					.setDescriptorCount(1)
					.setDstBinding(otherBinding)
					.setDstSet(*descriptorSet)
					.setSrcBinding(otherBinding)
					.setSrcSet(*m_vkDescriptorSets[oldMask]);

				descriptorSetCopys.push_back(copyDescriptorSet);
			}
		}
	}


	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, descriptorSetCopys);

	m_bindingAlignment[binding] = 0;
}

void RenderPass::bindResource(const std::string& name, IDynamicBufferResource& buffer)
{
	auto& dBuffer = dynamic_cast<DynamicBufferResource&>(buffer);
	auto& innerBuffer = dynamic_cast<BufferResource&>(*dBuffer.m_buffer);

	auto binding = getBinding(name);

	auto oldMask = m_descriptorSetKeyMask;
	m_descriptorSetKeyMask |= (0x01 << binding);

	//if we have not cached a descriptor set
	if (m_vkDescriptorSets.find(m_descriptorSetKeyMask) == m_vkDescriptorSets.end())
	{
		cacheNewPipeline(m_descriptorSetKeyMask);
	}


	auto& descriptorSet = m_vkDescriptorSets[m_descriptorSetKeyMask];

	vk::DescriptorBufferInfo info = innerBuffer.m_vkInfo;
	info.setRange(dBuffer.m_alignment);


	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
		.setDescriptorCount(1)
		.setPBufferInfo(&info);

	std::vector<vk::CopyDescriptorSet> descriptorSetCopys;
	//only copy "old" binding information (for other bindings than this) if we have just created a new descriptor set:
	if (oldMask != m_descriptorSetKeyMask) {
		for (auto& bindingAligment : m_bindingAlignment)
		{
			auto otherBinding = bindingAligment.first;

			if (otherBinding != binding) {
				auto copyDescriptorSet = vk::CopyDescriptorSet()
					.setDescriptorCount(1)
					.setDstBinding(otherBinding)
					.setDstSet(*descriptorSet)
					.setSrcBinding(otherBinding)
					.setSrcSet(*m_vkDescriptorSets[oldMask]);

				descriptorSetCopys.push_back(copyDescriptorSet);
			}
		}
	}

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, descriptorSetCopys);
	m_bindingAlignment[binding] = dBuffer.m_alignment;
}

void RenderPass::bindResource(const std::string & name, IImageResource &image, ISampler &sampler)
{
	auto& backendImage = dynamic_cast<ImageResource&>(image);
	auto& backendSampler = static_cast<Sampler&>(sampler);
	auto binding = getBinding(name);

	vk::DescriptorImageInfo info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*backendImage.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(backendSampler));

	auto& descriptorSet = m_vkDescriptorSets[m_descriptorSetKeyMask];
	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
	m_bindingAlignment[binding] = 0;
}

long RenderPass::getBinding(const std::string& name)
{
	//TODO: use method from renderpass
	long binding = -1;

	if (m_shaderProgram.m_vertexShader.bindingExists(name)) {
		binding = m_shaderProgram.m_vertexShader.m_bindings[name].binding;
	}
	else if (m_shaderProgram.m_fragmentShader.bindingExists(name)) {
		binding = m_shaderProgram.m_fragmentShader.m_bindings[name].binding;
	}
	else {
		PAPAGO_ERROR("Invalid uniform name!");
	}

	return binding;
};