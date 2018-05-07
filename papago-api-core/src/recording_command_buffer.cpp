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
long CommandRecorder<T>::getBinding(const std::string& name)
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
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("setUniform(name, image, sampler) called while not in a begin-context (begin(...) has not been called)");
	}

	auto& backendImage = static_cast<ImageResource&>(image);
	auto& backendSampler = static_cast<Sampler&>(sampler);
	auto binding = getBinding(uniformName);

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
};


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
T& CommandRecorder<T>::setUniform(const std::string& uniformName, DynamicBuffer& buffer, size_t	index)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("setUniform(uniformName, buffer) called while not in a begin-context (begin(...) has not been called)");
	}

	auto& innerBuffer = dynamic_cast<BufferResource&>(buffer.innerBuffer());

	auto binding = getBinding(uniformName);
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



//TODO: check if setUnifor(..., IBufferResource) is equal to setUniform(..., DynamicUniform)
template<class T>
T& CommandRecorder<T>::setUniform(const std::string & uniformName, IBufferResource & buffer)
{
	if (m_renderPassPtr == nullptr)
	{
		PAPAGO_ERROR("setUniform(uniformName, buffer) called while not in a begin-context (begin(...) has not been called)");
	}

	auto& backendBuffer = static_cast<BufferResource&>(buffer);
	auto binding = getBinding(uniformName);
	auto& descriptorSet = m_renderPassPtr->m_vkDescriptorSet;

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*descriptorSet)
		.setDstBinding(binding)
		.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
		.setDescriptorCount(1)
		.setPBufferInfo(&backendBuffer.m_vkInfo);

	m_vkDevice->updateDescriptorSets({ writeDescriptorSet }, {});

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

	m_bindingDynamicOffset[binding] = 0;

	for (auto i = 0; i < offsetCount; ++i) {
		dynamicOffsets[i] = m_bindingDynamicOffset[i];
	}

	m_vkCommandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_renderPassPtr->m_vkPipelineLayout, 0, { *descriptorSet }, dynamicOffsets);

	m_resourcesInUse.emplace(&backendBuffer);
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