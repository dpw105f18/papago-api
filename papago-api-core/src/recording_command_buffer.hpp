#pragma once

#include <set>
#include <map>

class IImageResource;
class ISampler;
class Resource;
class RenderPass;
class ShaderProgram;
class IBufferResource;
class DynamicBufferResource;
class CommandBuffer;
class IParameterBlock;

template<class T>
class CommandRecorder
	: public T
{
public:
	CommandRecorder(const vk::UniqueDevice& device) : m_vkDevice(device) 
	{
		m_dynamicBindings.reserve(64);	//<-- limit set by dynamic mask type.
	};
	
	
	CommandRecorder(CommandRecorder&& other)
		: m_vkDevice(other.m_vkDevice)
		, m_bindingDynamicOffset(std::move(other.m_bindingDynamicOffset))
		, m_renderPassPtr(other.m_renderPassPtr)
		, m_resourcesInUse(std::move(other.m_resourcesInUse))
		, m_vkCommandBuffer(std::move(other.m_vkCommandBuffer))
		, m_vkCommandPool(std::move(other.m_vkCommandPool))
		, m_dynamicBindings(std::move(other.m_dynamicBindings))
	{
	};

	virtual ~CommandRecorder() = default;

	// Inherited via IRecordingCommandBuffer
	T& setDynamicIndex(IParameterBlock& parameterBlock, const std::string& uniformName, size_t) override;

	std::map<uint32_t, uint32_t> m_bindingDynamicOffset;
	std::set<Resource*> m_resourcesInUse;
protected:
	//TODO: Check that this is not null, when calling non-begin methods on the object. - Brandborg
	// TODO: Another approach could be to create another interface and expose it via builder pattern or lambda expressions - CW 2018-04-23
	RenderPass* m_renderPassPtr;
	vk::UniqueCommandPool m_vkCommandPool;
	vk::UniqueCommandBuffer m_vkCommandBuffer;
	vk::RenderPassBeginInfo m_vkRenderPassBeginInfo;
	vk::Extent2D m_vkCurrentRenderTargetExtent;
	std::vector<uint32_t> m_dynamicBindings;
	std::vector<uint32_t> m_dynamicOffsets;



	const vk::UniqueDevice& m_vkDevice;

private:
};

//NOTE: method implementations given in .cpp file