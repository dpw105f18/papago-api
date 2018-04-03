#include "standard_header.hpp"
#include "command_buffer.hpp"
#include "swap_chain.hpp"
#include "render_pass.hpp"

CommandBuffer::operator vk::CommandBuffer&()
{
	return *m_vkCommandBuffer;
}

CommandBuffer::CommandBuffer(const vk::UniqueDevice &device, int queueFamilyIndex, Usage usage)
	: m_usage(usage)
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

//TODO: make checks to see if cmd.begin(...) has been called before. -AM
void CommandBuffer::begin(RenderPass & renderPass, SwapChain& swapChain)
{
	vk::Rect2D renderArea = {};
	renderArea.setOffset({ 0,0 })
		.setExtent(swapChain.m_vkExtent);

	vk::RenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*swapChain.m_framebuffers[swapChain.getCurrentFramebufferIndex()])
		.setRenderArea(renderArea)
		.setClearValueCount(1)
		.setPClearValues(&vk::ClearValue());

	//note: no clear-values because of the specific constructor overload...

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	//TODO: can we assume a graphics bindpoint and pipeline? -AM
	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPass.m_vkGraphicsPipeline);
}

void CommandBuffer::end()
{
	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}
