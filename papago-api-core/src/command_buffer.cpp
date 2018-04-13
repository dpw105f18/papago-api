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
		.setFramebuffer(*swapChain.m_framebuffers[imageIndex])
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

void CommandBuffer::setUniform(const std::string & name, const ImageResource & image, Sampler & sampler)
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

	vk::DescriptorImageInfo info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*image.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(sampler));

	auto writeDescriptorSet = vk::WriteDescriptorSet(*m_renderPassPtr->m_vkDescriptorSet, binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *m_renderPassPtr->m_vkDescriptorSet }, { });
}

void CommandBuffer::setInput(const BufferResource& buffer)
{
	//TODO: find a more general way to fix offsets
	VkDeviceSize offsets[] = { 0 };
	VkBuffer vertexBuffers[] = { *buffer.m_vkBuffer };

	//TODO: make it work with m_vkCommandBuffer->bindVertexBuffers(...);
	vkCmdBindVertexBuffers(*m_vkCommandBuffer, 0, 1, vertexBuffers, offsets);
}

void CommandBuffer::end()
{
	m_renderPassPtr = nullptr;
	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}
