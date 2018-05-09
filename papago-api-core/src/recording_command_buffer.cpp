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


