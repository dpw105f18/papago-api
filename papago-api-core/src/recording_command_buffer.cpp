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
long CommandRecorder<T>::getBinding(const ShaderProgram & program, const std::string& name)
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
};


template<class T>
T& CommandRecorder<T>::setUniform(const std::string& uniformName, IImageResource& image, ISampler& sampler)
{
	auto& backendImage = dynamic_cast<ImageResource&>(image);
	auto& backendSampler = dynamic_cast<Sampler&>(sampler);
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);

	vk::DescriptorImageInfo info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*backendImage.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(backendSampler));

	auto writeDescriptorSet = vk::WriteDescriptorSet(*m_renderPassPtr->m_vkDescriptorSet, binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *m_renderPassPtr->m_vkDescriptorSet }, {});

	m_resourcesInUse.emplace(&backendImage);
	return *this;
}

template<class T>
T& CommandRecorder<T>::setInput(IBufferResource& buffer)
{
	//TODO: find a more general way to fix offsets
	//TODO: make it work with m_vkCommandBuffer->bindVertexBuffers(...);
	m_vkCommandBuffer->bindVertexBuffers(
		0,
		{ *(static_cast<BufferResource&>(buffer)).m_vkBuffer },
		{ 0 });
	return *this;
};


template<class T>
T& CommandRecorder<T>::setIndexBuffer(IBufferResource &indexBuffer)
{
	// TODO: Retrieve wheter uint16 or uint32 is used for index buffer from somewhere - CW 2018-04-13
	m_vkCommandBuffer->bindIndexBuffer(
		*(dynamic_cast<BufferResource&>(indexBuffer)).m_vkBuffer,
		0,
		vk::IndexType::eUint16);
	return *this;
};

template<class T>
T& CommandRecorder<T>::setUniform(const std::string& uniformName, DynamicBuffer& buffer, size_t	index)
{
	auto& innerBuffer = dynamic_cast<BufferResource&>(buffer.innerBuffer());

	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	bool bindingAlreadyBound = false;
	for (auto boundBinding : CommandBuffer::s_boundDescriptorBindings) {
		bindingAlreadyBound = boundBinding == binding;

		if (bindingAlreadyBound) break;
	}

	if (!bindingAlreadyBound) {

		vk::DescriptorBufferInfo info = innerBuffer.m_vkInfo;
		info.setRange(buffer.m_alignment);

		auto writeDescriptorSet = vk::WriteDescriptorSet()
			.setDstSet(*descriptorSet)
			.setDstBinding(binding)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDescriptorCount(1)
			.setPBufferInfo(&info);

		m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});
		CommandBuffer::s_boundDescriptorBindings.push_back(binding);
	}

	/*
	"If any of the sets being bound include dynamic uniform or storage buffers,
	then pDynamicOffsets includes one element for each array element in each
	dynamic descriptor type binding in each set.

	Values are taken from pDynamicOffsets in an order such that:
	1) all entries for set N come before set N+1;
	2) within a set, entries are ordered by the binding numbers in the descriptor set layouts;
	3) and within a binding array, elements are in order.

	dynamicOffsetCount must equal the total number of dynamic descriptors in the sets being bound."

	From Vulkan Specification (edited format for clarity)
	https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkCmdBindDescriptorSets.html#descriptorsets-binding-dynamicoffsets
	*/


	std::set<uint32_t> uniqueBindings;
	auto& vertexBindings = m_renderPassPtr->m_shaderProgram.m_vertexShader.m_bindings;
	auto& fragmentBindings = m_renderPassPtr->m_shaderProgram.m_fragmentShader.m_bindings;

	for (auto& binding : vertexBindings)
	{
		uniqueBindings.insert(binding.second.binding);
	}

	for (auto& binding : fragmentBindings)
	{
		uniqueBindings.insert(binding.second.binding);
	}

	auto offsetCount = uniqueBindings.size();
	auto dynamicOffsets = std::vector<uint32_t>(offsetCount);

	m_bindingDynamicOffset[binding] = buffer.m_alignment * index;

	for (auto i = 0; i < offsetCount; ++i) {
		dynamicOffsets[i] = m_bindingDynamicOffset[i];
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, dynamicOffsets);

	return *this;
};



template<class T>
T& CommandRecorder<T>::setUniform(const std::string & uniformName, IBufferResource & buffer)
{
	auto& backendBuffer = dynamic_cast<BufferResource&>(buffer);
	auto binding = getBinding(m_renderPassPtr->m_shaderProgram, uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
		.setDescriptorCount(1)
		.setPBufferInfo(&backendBuffer.m_vkInfo);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});

	// TODO: Find the amount of dynamic offsets that is required by the number of dynamic uniform buffers
	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, { 0,0 });

	m_resourcesInUse.emplace(&backendBuffer);
	return *this;
};



template<class T>
T& CommandRecorder<T>::drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance)
{
	m_vkCommandBuffer->drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	return *this;
};