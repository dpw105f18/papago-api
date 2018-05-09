#pragma once

#include <set>
#include <map>

class IImageResource;
class ISampler;
class Resource;
class RenderPass;
class ShaderProgram;
class IBufferResource;
class DynamicBuffer;
class CommandBuffer;

template<class T>
class CommandRecorder
	: public T
{
public:
	CommandRecorder(const vk::UniqueDevice& device) : m_vkDevice(device) {};
	CommandRecorder(CommandRecorder&& other)
		: m_vkDevice(other.m_vkDevice)
		, m_bindingDynamicOffset(std::move(other.m_bindingDynamicOffset))
		, m_renderPassPtr(other.m_renderPassPtr)
		, m_resourcesInUse(std::move(other.m_resourcesInUse))
		, m_vkCommandBuffer(std::move(other.m_vkCommandBuffer))
		, m_vkCommandPool(std::move(other.m_vkCommandPool))
	{};

	virtual ~CommandRecorder() = default;

	// Inherited via IRecordingCommandBuffer
	T& setInput(IBufferResource &) override;
	T& setUniform(const std::string & uniformName, IImageResource &, ISampler &) override;
	T& setUniform(const std::string & uniformName, IBufferResource &) override;
	T& setUniform(const std::string & uniformName, DynamicBuffer &, size_t) override;
	T& setIndexBuffer(IBufferResource &) override;
	T& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) override;

	T& clearColorBuffer(float red, float green, float blue, float alpha) override;
	T& clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha) override;
	T& clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) override;
	T& clearDepthStencilBuffer(float depth, uint32_t stencil) override;
	T& clearDepthBuffer(float value) override;
	T& clearStencilBuffer(uint32_t value) override;

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

	CommandBuffer* m_state;

	const vk::UniqueDevice& m_vkDevice;

private:

	void clearAttachment(const vk::ClearValue& clearValue, vk::ImageAspectFlags aspectFlags);
};

//NOTE: method implementations given in .cpp file