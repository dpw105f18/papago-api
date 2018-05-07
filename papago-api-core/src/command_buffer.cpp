#include "standard_header.hpp"
#include "command_buffer.hpp"
#include "swap_chain.hpp"
#include "render_pass.hpp"
#include "sampler.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "sub_command_buffer.hpp"
#include "ibuffer_resource.hpp"
#include "recording_command_buffer.cpp" //<-- resolves linker issues. -AM -- see: https://www.codeproject.com/Articles/48575/How-to-define-a-template-class-in-a-h-file-and-imp 

CommandBuffer::operator vk::CommandBuffer&()
{
	return *m_vkCommandBuffer;
}

IRecordingCommandBuffer & CommandBuffer::execute(std::vector<std::unique_ptr<ISubCommandBuffer>>& subCommands)
{
	//TODO: check subCommands to see if they are ready to be executed? -AM

	auto secondaryCommandBuffers = std::vector<vk::CommandBuffer>();
	secondaryCommandBuffers.reserve(subCommands.size());

	for (auto& isub : subCommands) {
		auto& internalSub = dynamic_cast<SubCommandBuffer&>(*isub);
		secondaryCommandBuffers.push_back(static_cast<vk::CommandBuffer>(internalSub));
	}
	
	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->beginRenderPass(m_vkRenderPassBeginInfo, vk::SubpassContents::eSecondaryCommandBuffers);

	m_vkCommandBuffer->executeCommands(secondaryCommandBuffers);

	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->beginRenderPass(m_vkRenderPassBeginInfo, vk::SubpassContents::eInline);

	return *this;
}

CommandBuffer::CommandBuffer(const vk::UniqueDevice &device, int queueFamilyIndex, Usage usage)
	: CommandRecorder<IRecordingCommandBuffer>(device), m_usage(usage), m_queueFamilyIndex(queueFamilyIndex)
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

CommandBuffer::CommandBuffer(CommandBuffer &&other)
	: CommandRecorder<IRecordingCommandBuffer>(std::move(other))
{
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

std::unique_ptr<ISubCommandBuffer> CommandBuffer::createSubCommandBuffer()
{
	return std::make_unique<SubCommandBuffer>(m_vkDevice, m_queueFamilyIndex);
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

	m_vkRenderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*swapChain.m_vkFramebuffers[imageIndex])
		.setRenderArea(renderArea)
		.setClearValueCount(clearValues.size())
		.setPClearValues(clearValues.data());

	//note: no clear-values because of the specific constructor overload...

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(m_vkRenderPassBeginInfo, vk::SubpassContents::eInline);

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

	
	m_vkRenderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*fbo)
		.setRenderArea(renderArea)
		.setClearValueCount(clearValues.size())
		.setPClearValues(clearValues.data());

	//note: no clear-values because of the specific constructor overload...

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(m_vkRenderPassBeginInfo, vk::SubpassContents::eInline);

	//TODO: can we assume a graphics bindpoint and pipeline? -AM
	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPass.m_vkGraphicsPipeline);
}

void CommandBuffer::end()
{
	m_renderPassPtr = nullptr;
	m_vkRenderPassBeginInfo = {};

	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}

//static:
std::vector<uint32_t> CommandBuffer::s_boundDescriptorBindings;