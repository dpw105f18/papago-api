#pragma once
#include <vector>

class Resource
{
public:
	Resource(Resource&& other) noexcept;
	Resource(const Resource&) = delete;

	virtual ~Resource() = default;
protected:
	explicit Resource(const vk::UniqueDevice& device);

	Resource(
		const vk::PhysicalDevice& physicalDevice,
		const vk::UniqueDevice& device,
		vk::MemoryPropertyFlags flags,
		vk::MemoryRequirements memoryRequirements);

	vk::UniqueDeviceMemory m_vkMemory;
	const vk::UniqueDevice& m_vkDevice;
	static uint32_t findMemoryType(
		const vk::PhysicalDevice&, 
		uint32_t memoryTypeBits, 
		const vk::MemoryPropertyFlags&);

	//TODO: make m_size the actual size of the resource, not the alligned size. -AM
	size_t m_size;
	vk::Fence* m_vkFence;

private:
	friend class GraphicsQueue;
};
