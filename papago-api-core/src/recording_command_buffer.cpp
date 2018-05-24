#include "standard_header.hpp"
#include "recording_command_buffer.hpp"


#include "render_pass.hpp"
#include "buffer_resource.hpp"
#include "parameter_block.hpp"


template<class T>
T & CommandRecorder<T>::setDynamicIndex(IParameterBlock& parameterBlock, const std::string & uniformName, size_t index)
{
	auto& internalParameterBlock = dynamic_cast<ParameterBlock&>(parameterBlock);
	auto dynamicBufferMask = internalParameterBlock.m_mask;
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
	//TODO: check [name] is actually in the map before trying to access the alignment. -AM
	m_bindingDynamicOffset[binding] = internalParameterBlock.m_namedAlignments[uniformName] * index;

	std::sort(dynamicBindings.begin(), dynamicBindings.end());

	for (auto b : dynamicBindings) {
		dynamicOffsets.push_back(m_bindingDynamicOffset[b]);
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayouts[internalParameterBlock.m_mask], 0, { *internalParameterBlock.m_vkDescriptorSet }, dynamicOffsets);
	return *this;
}
;



