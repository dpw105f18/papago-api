#pragma once
#include <vector>
#include "standard_header.hpp"

class Resource
{
public:
	Resource(Resource&& other) noexcept;
	Resource(const Resource&) = delete;

	virtual ~Resource() = default;
	virtual void destroy() = 0;
	virtual void upload(const std::vector<char>& data);
	std::vector<char> download();
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

	size_t m_size;
	vk::Fence* m_vkFence;

private:
	friend class GraphicsQueue;
};
