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
T & CommandRecorder<T>::setDynamicIndex(const std::string & uniformName, size_t index)
{
	auto dynamicBufferMask = m_renderPassPtr->m_descriptorSetKeyMask;
	auto bindingCount = m_renderPassPtr->m_shaderProgram.getUniqueUniformBindings().size();

	uint64_t longOne = 0x01;
	std::vector<uint32_t> dynamicBindings;
	for (auto i = 0; i < 64; ++i) {
		if (dynamicBufferMask & (longOne << i)) {
			dynamicBindings.push_back(i);
		}
	}

	auto dynamicOffsets = std::vector<uint32_t>();

	auto binding = m_renderPassPtr->getBinding(uniformName);
	m_bindingDynamicOffset[binding] = m_renderPassPtr->m_bindingAlignment[binding] * index;

	std::vector<uint32_t> bindings;

	for (auto& dynamicBindingOffset : m_bindingDynamicOffset)
	{
		bindings.push_back(dynamicBindingOffset.first);
	}

	std::sort(bindings.begin(), bindings.end());

	for (auto b : bindings) {
		dynamicOffsets.push_back(m_bindingDynamicOffset[b]);
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayouts[m_renderPassPtr->m_descriptorSetKeyMask], 0, { *m_renderPassPtr->m_vkDescriptorSets[m_renderPassPtr->m_descriptorSetKeyMask] }, dynamicOffsets);
	return *this;
}
;



