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
	auto& internalSwapChain = static_cast<SwapChain&>(swapchain);
	begin(static_cast<RenderPass&>(renderPass), internalSwapChain.m_vkFramebuffers[frameIndex], { swapchain.getWidth(), swapchain.getHeight() });
	func(*this);
	end();
}

void CommandBuffer::record(IRenderPass & renderPass, IImageResource & target, std::function<void(IRecordingCommandBuffer&)> func)
{
	auto& internalColor = static_cast<ImageResource&>(target);
	auto& internalRenderPass = static_cast<RenderPass&>(renderPass);
	auto extent = internalColor.m_vkExtent;

	vk::ImageView attachments[1] = { *internalColor.m_vkImageView };

	vk::FramebufferCreateInfo fboCreate;
	fboCreate.setAttachmentCount(1)
		.setPAttachments(attachments)
		.setWidth(extent.width)
		.setHeight(extent.height)
		.setLayers(1)
		.setRenderPass(static_cast<vk::RenderPass>(internalRenderPass));

	//TODO: Find out if the framebuffer should reside on the image like this. Maybe commandbuffer instea? - Brandborg
	internalColor.m_vkFramebuffer = m_vkDevice->createFramebufferUnique(fboCreate);

	begin(internalRenderPass, internalColor.m_vkFramebuffer, { extent.width, extent.height });
	func(*this);
	end();
}

void CommandBuffer::record(IRenderPass& renderPass, IImageResource& color, IImageResource& depth, std::function<void(IRecordingCommandBuffer&)> func)
{
	auto& internalColor = static_cast<ImageResource&>(color);
	auto& internalDepth = static_cast<ImageResource&>(depth);
	auto& internalRenderPass = static_cast<RenderPass&>(renderPass);
	auto extent = internalColor.m_vkExtent;
	vk::ImageView attachments[2] = { *internalColor.m_vkImageView, *internalDepth.m_vkImageView };

	vk::FramebufferCreateInfo fboCreate;
	fboCreate.setAttachmentCount(2)
		.setPAttachments(attachments)
		.setWidth(extent.width)
		.setHeight(extent.height)
		.setLayers(1)
		.setRenderPass(static_cast<vk::RenderPass>(internalRenderPass));

	internalColor.m_vkFramebuffer = m_vkDevice->createFramebufferUnique(fboCreate);

	begin(internalRenderPass, internalColor.m_vkFramebuffer, { extent.width, extent.height });
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

IRecordingCommandBuffer & CommandBuffer::clearColorBuffer(float red, float green, float blue, float alpha)
{
	auto colorArray = std::array<float, 4>{ red,green,blue,alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttatchment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}

IRecordingCommandBuffer & CommandBuffer::clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha)
{
	auto colorArray = std::array<int32_t, 4>{ red, green, blue, alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttatchment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}

IRecordingCommandBuffer & CommandBuffer::clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
	auto colorArray = std::array<uint32_t, 4>{ red, green, blue, alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttatchment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}

IRecordingCommandBuffer & CommandBuffer::clearDepthStencilBuffer(float depth, uint32_t stencil)
{
	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == (DepthStencilFlags::eDepth | DepthStencilFlags::eStencil)) {
			auto color = vk::ClearDepthStencilValue({ depth, stencil });
			clearAttatchment(color, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
		}
		else {
			PAPAGO_ERROR("Tried to clear buffer which is not both depth and stencil!");
		}
	}
	else {
		PAPAGO_ERROR("Tried to clear non-existent depth/stencil buffer!");
	}
	return *this;
}

IRecordingCommandBuffer& CommandBuffer::clearDepthBuffer(float value)
{
	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == DepthStencilFlags::eDepth) {
			auto color = vk::ClearDepthStencilValue(value);
			clearAttatchment(color, vk::ImageAspectFlagBits::eDepth);
		}
		else {
			PAPAGO_ERROR("Tried to clear buffer which is not depth!");
		}
	}
	else {
		PAPAGO_ERROR("Tried to clear non-existent depth/stencil buffer!");
	}
	return *this;
}

IRecordingCommandBuffer& CommandBuffer::clearStencilBuffer(uint32_t value)
{
	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == DepthStencilFlags::eStencil) {
			auto color = vk::ClearDepthStencilValue(0, value);
			clearAttatchment(color, vk::ImageAspectFlagBits::eStencil);
		}
		else {
			PAPAGO_ERROR("Tried to clear buffer which is not stencil!");
		}
	}
	else {
		PAPAGO_ERROR("Tried to clear non-existent depth/stencil buffer!");
	}
	return *this;
}

void CommandBuffer::clearAttatchment(const vk::ClearValue& clearValue, vk::ImageAspectFlags aspectFlags)
{
	vk::ClearAttachment clearInfo = {};
	clearInfo.setAspectMask(aspectFlags)
		.setColorAttachment(0) // As we only have a single color attatchment, it will always be at 0. Ignored if depth/stencil.
		.setClearValue(clearValue);

	vk::ClearRect clearRect = {};
	vk::Rect2D rect = { { 0, 0 },{ m_vkCurrentRenderTargetExtent.width, m_vkCurrentRenderTargetExtent.height } };
	clearRect.setRect(rect).setBaseArrayLayer(0).setLayerCount(1);
	m_vkCommandBuffer->clearAttachments(clearInfo, { clearRect });
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

void CommandBuffer::begin(RenderPass& renderPass, const vk::UniqueFramebuffer& renderTarget, vk::Extent2D extent)
{
	m_renderPassPtr = &renderPass;
	m_vkCurrentRenderTargetExtent = extent;

	vk::Rect2D renderArea = {};
	renderArea.setOffset({ 0,0 })
		.setExtent(extent);

	vk::RenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.setRenderPass(static_cast<vk::RenderPass>(renderPass))
		.setFramebuffer(*renderTarget)
		.setRenderArea(renderArea)
		.setClearValueCount(0)
		.setPClearValues(nullptr);

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);	//TODO: read from Usage in constructor? -AM

	m_vkCommandBuffer->begin(beginInfo);
	m_vkCommandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	//TODO: can we assume a graphics bindpoint and pipeline? -AM
	m_vkCommandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *renderPass.m_vkGraphicsPipeline);
}

void CommandBuffer::end()
{
	m_renderPassPtr = nullptr;
	m_vkCurrentRenderTargetExtent = vk::Extent2D();
	m_vkCommandBuffer->endRenderPass();
	m_vkCommandBuffer->end();
}

IRecordingCommandBuffer& CommandBuffer::setIndexBuffer(IBufferResource &indexBuffer)
{
	// TODO: Retrieve wheter uint16 or uint32 is used for index buffer from somewhere - CW 2018-04-13
	m_vkCommandBuffer->bindIndexBuffer(
		*(static_cast<BufferResource&>(indexBuffer)).m_vkBuffer, 
		0, 
		vk::IndexType::eUint16);
	return *this;
}

void CommandBuffer::drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation)
{
	//TODO: choose the correct draw-command based on how the buffer has been used? -AM
	m_vkCommandBuffer->draw(instanceVertexCount, instanceCount, startVertexLocation, startInstanceLocation);
}

IRecordingCommandBuffer& CommandBuffer::setUniform(const std::string & uniformName, IBufferResource & buffer)
{
	auto& backendBuffer = static_cast<BufferResource&>(buffer);
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&backendBuffer.m_vkInfo);

	m_vkDevice->updateDescriptorSets({writeDescriptorSet}, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, {});

	m_resourcesInUse.emplace(&backendBuffer);
	return *this;
}


IRecordingCommandBuffer& CommandBuffer::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return *this;
}
