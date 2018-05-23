#include "standard_header.hpp"
#include "sub_command_buffer.hpp"
#include "render_pass.hpp"
#include "swap_chain.hpp"
#include "image_resource.hpp"
#include "ibuffer_resource.hpp"
#include "recording_command_buffer.cpp" //<-- resolves linker issues. -AM
#include "parameter_block.hpp"

SubCommandBuffer::SubCommandBuffer(const vk::UniqueDevice& device, uint32_t queueFamilyIndex)
	: CommandRecorder<IRecordingSubCommandBuffer>(device)
{
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

	if (m_renderPassPtr->m_shaderProgram.getUniqueUniformBindings().empty()) {
		m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->getPipeline(0));
	}
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

IRecordingSubCommandBuffer & SubCommandBuffer::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("drawIndexed(...) called while not in a begin-context (begin(...) has not been called)");
	}

	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return *this;
}

IRecordingSubCommandBuffer & SubCommandBuffer::draw(size_t vertexCount, size_t instanceCount, size_t firstVertex, size_t firstInstance)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("drawIndexed(...) called while not in a begin-context (begin(...) has not been called)");
	}

	m_vkCommandBuffer->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

IRecordingSubCommandBuffer & SubCommandBuffer::setVertexBuffer(IBufferResource &buffer)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("setInput(buffer) called while not in a begin-context (begin(...) has not been called)");
	}

	//TODO: find a more general way to fix offsets
	m_vkCommandBuffer->bindVertexBuffers(
		0,
		{ *(static_cast<BufferResource&>(buffer)).m_vkBuffer },
		{ 0 });
	return *this;
}

IRecordingSubCommandBuffer & SubCommandBuffer::setIndexBuffer(IBufferResource &indexBuffer)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("setIndexBuffer(indexBuffer) called while not in a begin-context (begin(...) has not been called)");
	}

	auto& internalIndexBuffer = static_cast<BufferResource&>(indexBuffer);
	auto indexType = internalIndexBuffer.m_elementType == BufferResourceElementType::eUint32 ? vk::IndexType::eUint32 : vk::IndexType::eUint16;

	m_vkCommandBuffer->bindIndexBuffer(
		*internalIndexBuffer.m_vkBuffer,
		0,
		indexType);
	return *this;
}
IRecordingSubCommandBuffer & SubCommandBuffer::setParameterBlock(IParameterBlock& parameterBlock)
{
	auto& internalParameterBlock = dynamic_cast<ParameterBlock&>(parameterBlock);
	auto& pipeline = m_renderPassPtr->getPipeline(internalParameterBlock.m_mask);
	auto& layout = m_renderPassPtr->getPipelineLayout(internalParameterBlock.m_mask);

	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
	m_vkCommandBuffer->bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, 
		*layout, 
		0, 
		{ *internalParameterBlock.m_vkDescriptorSet }, 
		std::vector<uint32_t>(internalParameterBlock.m_dynamicBufferCount)
	);
	
	return *this;
}
;
