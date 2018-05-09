#include "standard_header.hpp"
#include "recording_command_buffer.hpp"


#include "render_pass.hpp"
#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "image_resource.hpp"
#include "sampler.hpp"
#include "buffer_resource.hpp"

#include <heapapi.h>


template<class T>
void CommandRecorder<T>::clearAttachment(const vk::ClearValue & clearValue, vk::ImageAspectFlags aspectFlags)
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

template<class T>
T& CommandRecorder<T>::setInput(IBufferResource& buffer)
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
template<class T>
T & CommandRecorder<T>::setDynamicIndex(const std::string & uniformName, size_t index)
{
	//IMPROVEMENT: cache this so we don't need to loop over bindings every time we set an index. -AM
	std::set<uint32_t> uniqueBindings;

	auto& vBindings = m_renderPassPtr->m_shaderProgram.m_vertexShader.getBindings();
	auto& fBindings = m_renderPassPtr->m_shaderProgram.m_fragmentShader.getBindings();

	for (auto& vb : vBindings) {
		uniqueBindings.insert(vb.binding);
	}

	for (auto& fb : fBindings) {
		uniqueBindings.insert(fb.binding);
	}

	auto offsetCount = uniqueBindings.size();
	auto dynamicOffsets = std::vector<uint32_t>(offsetCount);

	auto binding = m_renderPassPtr->getBinding(uniformName);
	m_bindingDynamicOffset[binding] = m_renderPassPtr->m_bindingAlignment[binding] * index;

	for (auto i = 0; i < offsetCount; ++i) {
		dynamicOffsets[i] = m_bindingDynamicOffset[i];
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *m_renderPassPtr->m_vkDescriptorSet }, dynamicOffsets);
	return *this;
}
;


template<class T>
T& CommandRecorder<T>::setIndexBuffer(IBufferResource &indexBuffer)
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
};

template<class T>
T& CommandRecorder<T>::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("drawIndexed(...) called while not in a begin-context (begin(...) has not been called)");
	}

	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return *this;
}

template<class T>
T & CommandRecorder<T>::draw(size_t vertexCount, size_t instanceCount, size_t firstVertex, size_t firstInstance)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("drawIndexed(...) called while not in a begin-context (begin(...) has not been called)");
	}

	m_vkCommandBuffer->draw(vertexCount, instanceCount, firstVertex, firstInstance);
}


template<class T>
T & CommandRecorder<T>::clearColorBuffer(float red, float green, float blue, float alpha)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearColorBuffer(float red...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto colorArray = std::array<float, 4>{ red, green, blue, alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttachment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}
template<class T>
T & CommandRecorder<T>::clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearColorBuffer(int32_t red...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto colorArray = std::array<int32_t, 4>{ red, green, blue, alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttachment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}
template<class T>
T & CommandRecorder<T>::clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearColorBuffer(uint32_t red...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto colorArray = std::array<uint32_t, 4>{ red, green, blue, alpha };
	auto color = vk::ClearColorValue(colorArray);
	clearAttachment(color, vk::ImageAspectFlagBits::eColor);
	return *this;
}
template<class T>
T & CommandRecorder<T>::clearDepthStencilBuffer(float depth, uint32_t stencil)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearDepthStencilBuffer(...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == (DepthStencilFlags::eDepth | DepthStencilFlags::eStencil)) {
			auto color = vk::ClearDepthStencilValue({ depth, stencil });
			clearAttachment(color, vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
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
template<class T>
T & CommandRecorder<T>::clearDepthBuffer(float value)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearDepthBuffer(...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == DepthStencilFlags::eDepth) {
			auto color = vk::ClearDepthStencilValue(value);
			clearAttachment(color, vk::ImageAspectFlagBits::eDepth);
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
template<class T>
T & CommandRecorder<T>::clearStencilBuffer(uint32_t value)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("clearStencilBuffer(...) called while not in a begin-context (begin(...) has not been called)");
	}

	auto flags = m_renderPassPtr->m_depthStencilFlags;
	if (flags != DepthStencilFlags::eNone) {
		if (flags == DepthStencilFlags::eStencil) {
			auto color = vk::ClearDepthStencilValue(0, value);
			clearAttachment(color, vk::ImageAspectFlagBits::eStencil);
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
;