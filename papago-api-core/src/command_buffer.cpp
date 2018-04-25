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

RecordingCommandBuffer& CommandBuffer::setUniform(const std::string & name, IImageResource & image, ISampler & sampler)
{
	auto& i = (ImageResource&)image;
	auto& s = (Sampler&)sampler;
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, name);

	vk::DescriptorImageInfo info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*i.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(s));

	auto writeDescriptorSet = vk::WriteDescriptorSet(*m_renderPassPtr->m_vkDescriptorSet, binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *m_renderPassPtr->m_vkDescriptorSet }, { });

	m_resourcesInUse.emplace(&i);
}

void CommandBuffer::setInput(const BufferResource& buffer)
{
	//TODO: find a more general way to fix offsets
	//TODO: make it work with m_vkCommandBuffer->bindVertexBuffers(...);
	m_vkCommandBuffer->bindVertexBuffers(0, { *buffer.m_vkBuffer }, { 0 });
}

void CommandBuffer::end()
{
	m_renderPassPtr = nullptr;
	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

void CommandBuffer::setIndexBuffer(const BufferResource &indexBuffer)
{
	// TODO: Retrieve wheter uint16 or uint32 is used for index buffer from somewhere - CW 2018-04-13
	m_vkCommandBuffer->bindIndexBuffer(*indexBuffer.m_vkBuffer, 0, vk::IndexType::eUint16);
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}

RecordingCommandBuffer& CommandBuffer::setUniform(const std::string & uniformName, IBufferResource & buffer)
{
	auto& b = (BufferResource&)buffer;
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&b.m_vkInfo);

	m_vkDevice->updateDescriptorSets({writeDescriptorSet}, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, {});

	m_resourcesInUse.emplace(&b);
	return *this;
}


RecordingCommandBuffer& CommandBuffer::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
