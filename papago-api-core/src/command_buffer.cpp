#include "standard_header.hpp"
#include "command_buffer.hpp"
#include "swap_chain.hpp"
#include "render_pass.hpp"
#include "sampler.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"

CommandBuffer::operator vk::CommandBuffer&()
{
	return *m_vkCommandBuffer;
}

CommandBuffer::CommandBuffer(const vk::UniqueDevice &device, int queueFamilyIndex, Usage usage)
	: m_usage(usage), m_vkDevice(device)
{
	vk::CommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	m_vkCommandPool = device->createCommandPoolUnique(poolCreateInfo);

	vk::CommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.setCommandBufferCount(1)	//TODO: find good way to make setable. -AM
		.setCommandPool(*m_vkCommandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	m_vkCommandBuffer = std::move(device->allocateCommandBuffersUnique(allocateInfo)[0]);	//TODO: remove "[0]"-hack. -AM
}

void CommandBuffer::record(IRenderPass & renderPass, ISwapchain & swapchain, size_t frameIndex, std::function<void(IRecordingCommandBuffer&)> func)
{
	begin(static_cast<RenderPass&>(renderPass), static_cast<SwapChain&>(swapchain), frameIndex);
	func(*this);
	end();
}

void CommandBuffer::record(IRenderPass & renderPass, IImageResource & target, std::function<void(IRecordingCommandBuffer&)> func)
{
	begin(static_cast<RenderPass&>(renderPass), static_cast<ImageResource&>(target));
	func(*this);
	end();
}

long CommandBuffer::getBinding(const ShaderProgram & program, const std::string& name)
{
	long binding = -1;
	auto& shaderProgram = m_renderPassPtr->m_shaderProgram;

	if (shaderProgram.m_vertexShader.bindingExists(name)) {
		binding = shaderProgram.m_vertexShader.m_bindings[name].binding;
	}
	else if (shaderProgram.m_fragmentShader.bindingExists(name)) {
		binding = shaderProgram.m_fragmentShader.m_bindings[name].binding;
	}
	else {
		PAPAGO_ERROR("Invalid uniform name!");
	}

	return binding;
}

//TODO: make checks to see if cmd.begin(...) has been called before. -AM
void CommandBuffer::begin(RenderPass& renderPass, SwapChain& swapChain, uint32_t imageIndex)
{
	m_renderPassPtr = &renderPass;

	vk::Rect2D renderArea = {};
	renderArea.setOffset({ 0,0 })
		.setExtent(swapChain.m_vkExtent);


	std::array<vk::ClearValue, 2> clearValues;

	clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>{0, 0, 0, 1}));
	clearValues[1].setDepthStencil(vk::ClearDepthStencilValue{ 1.0, 0 });

	vk::RenderPassBeginInfo renderPassBeginInfo = {};

	renderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*swapChain.m_vkFramebuffers[imageIndex])
		.setRenderArea(renderArea)
		.setClearValueCount(clearValues.size())
		.setPClearValues(clearValues.data());

	//note: no clear-values because of the specific constructor overload...

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	//TODO: can we assume a graphics bindpoint and pipeline? -AM
	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPass.m_vkGraphicsPipeline);
}

void CommandBuffer::begin(RenderPass &renderPass, ImageResource & renderTarget)
{
	m_renderPassPtr = &renderPass;

	vk::Rect2D renderArea = {};
	renderArea.setOffset({ 0,0 })
		.setExtent({renderTarget.m_vkExtent.width, renderTarget.m_vkExtent.height});


	std::array<vk::ClearValue, 2> clearValues;

	clearValues[0].setColor(vk::ClearColorValue(std::array<float, 4>{1, 0, 0, 1}));
	clearValues[1].setDepthStencil(vk::ClearDepthStencilValue{ 1.0, 0 });

	auto& fbo = renderTarget.createFramebuffer(static_cast<vk::RenderPass>(renderPass));

	vk::RenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*fbo)
		.setRenderArea(renderArea)
		.setClearValueCount(clearValues.size())
		.setPClearValues(clearValues.data());

	//note: no clear-values because of the specific constructor overload...

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	//TODO: can we assume a graphics bindpoint and pipeline? -AM
	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPass.m_vkGraphicsPipeline);
}

IRecordingCommandBuffer& CommandBuffer::setUniform(const std::string & name, IImageResource & image, ISampler & sampler)
{
	auto& backendImage = static_cast<ImageResource&>(image);
	auto& backendSampler = static_cast<Sampler&>(sampler);
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, name);

	vk::DescriptorImageInfo info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*backendImage.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(backendSampler));

	auto writeDescriptorSet = vk::WriteDescriptorSet(*m_renderPassPtr->m_vkDescriptorSet, binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *m_renderPassPtr->m_vkDescriptorSet }, { });

	m_resourcesInUse.emplace(&backendImage);
	return *this;
}

IRecordingCommandBuffer& CommandBuffer::setInput(IBufferResource& buffer)
{
	//TODO: find a more general way to fix offsets
	//TODO: make it work with m_vkCommandBuffer->bindVertexBuffers(...);
	m_vkCommandBuffer->bindVertexBuffers(
		0, 
		{ *(static_cast<BufferResource&>(buffer)).m_vkBuffer }, 
		{ 0 });
	return *this;
}

void CommandBuffer::end()
{
	m_renderPassPtr = nullptr;
	m_boundDescriptorBindings.clear();

	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

IRecordingCommandBuffer& CommandBuffer::setIndexBuffer(IBufferResource &indexBuffer)
{
	// TODO: Retrieve wheter uint16 or uint32 is used for index buffer from somewhere - CW 2018-04-13
	m_vkCommandBuffer->bindIndexBuffer(
		*(dynamic_cast<BufferResource&>(indexBuffer)).m_vkBuffer, 
		0, 
		vk::IndexType::eUint16);
	return *this;
}

IRecordingCommandBuffer& CommandBuffer::setUniform(
	const std::string&	uniformName,
	DynamicBuffer&		buffer,
	size_t				index)
{
	auto& innerBuffer = dynamic_cast<BufferResource&>(buffer.innerBuffer());
	
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	bool bindingAlreadyBound = false;
	for (auto boundBinding : m_boundDescriptorBindings) {
		bindingAlreadyBound = boundBinding == binding;

		if (bindingAlreadyBound) break;
	}

	if (!bindingAlreadyBound) {

		auto writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*descriptorSet)
			.setDstBinding(binding)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(1)
			.setPBufferInfo(&innerBuffer.m_vkInfo);

		m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
		m_boundDescriptorBindings.push_back(binding);
	}
	// TODO: Find the amount of dynamic offsets that is required by the number of dynamic uniform buffers

	auto dynamicOffsets = std::vector<uint32_t>(buffer.m_objectCount);
	
	for (auto i = 0; i < buffer.m_objectCount; ++i) {
		if (i == index) {
			dynamicOffsets[i] = i * buffer.m_alignment;
		}
		else {
			//TODO: find some way to get the dynamic offsets of the uniforms we are NOT setting with this method. -AM
			//HACK: using 0 as a placeholder value for dynamic offsets
			dynamicOffsets[i] = 0;
		}
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, dynamicOffsets);

	return *this;
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}

IRecordingCommandBuffer& CommandBuffer::setUniform(const std::string & uniformName, IBufferResource & buffer)
{
	auto& backendBuffer = dynamic_cast<BufferResource&>(buffer);
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	auto writeDescriptorSet = vk::WriteDescriptorSet()
	.setDstSet(*descriptorSet)
	.setDstBinding(binding)
	.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
	.setDescriptorCount(1)
	.setPBufferInfo(&backendBuffer.m_vkInfo);

	m_vkDevice->updateDescriptorSets({writeDescriptorSet}, {});

	// TODO: Find the amount of dynamic offsets that is required by the number of dynamic uniform buffers
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, {0,0}); 

	m_resourcesInUse.emplace(&backendBuffer);
	return *this;
}


IRecordingCommandBuffer& CommandBuffer::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return *this;
}
