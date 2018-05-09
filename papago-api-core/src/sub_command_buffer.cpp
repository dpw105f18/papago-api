#include "standard_header.hpp"
#include "sub_command_buffer.hpp"
#include "render_pass.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"
#include "ibuffer_resource.hpp"
#include "recording_command_buffer.cpp" //<-- resolves linker issues. -AM

SubCommandBuffer::SubCommandBuffer(const vk::UniqueDevice& device, uint32_t queueFamilyIndex, CommandBuffer* parent)
	: CommandRecorder<IRecordingSubCommandBuffer>(device)
{
	m_state = parent;
	vk::CommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.setQueueFamilyIndex(queueFamilyIndex)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	m_vkCommandPool = device->createCommandPoolUnique(poolCreateInfo);

	vk::CommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.setCommandBufferCount(1)	//TODO: find good way to make setable. -AM
		.setCommandPool(*m_vkCommandPool)
		.setLevel(vk::CommandBufferLevel::eSecondary);

	m_vkCommandBuffer = std::move(device->allocateCommandBuffersUnique(allocateInfo)[0]);
}

SubCommandBuffer::operator vk::CommandBuffer&()
{
	return *m_vkCommandBuffer;
}

void SubCommandBuffer::begin()
{
	vk::CommandBufferInheritanceInfo inheritInfo = {};
	inheritInfo.occlusionQueryEnable = VK_FALSE;

	inheritInfo.renderPass = *m_renderPassPtr->m_vkRenderPass;

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse;
	beginInfo.pInheritanceInfo = &inheritInfo;

	m_vkCommandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);	//TODO: have usage and reset (or not) accordingly. -AM
	m_vkCommandBuffer->begin(beginInfo);

	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkGraphicsPipeline);
}


void SubCommandBuffer::end()
{
	m_vkCommandBuffer->end();
}


void SubCommandBuffer::record(IRenderPass &renderPass, std::function<void(IRecordingSubCommandBuffer&)> func)
{
	m_renderPassPtr = reinterpret_cast<RenderPass*>(&renderPass);
	begin();
	func(*this);
	end();
}
