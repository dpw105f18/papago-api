#pragma once
#include "resource.hpp"
#include "device.hpp"

class BufferResource : public Resource
{
public:
	// Inherited via Resource
	void upload(std::vector<char> data) override;
	void destroy() override;
	void* download(void * destination, size_t size, size_t offset) override;

	size_t getSize() const { return m_size; }

private:
	vk::UniqueBuffer m_vkBuffer;
	vk::UniqueDeviceMemory m_vkMemory;
	const vk::UniqueDevice& m_vkDevice;
	const vk::PhysicalDevice& m_vkPhysicalDevice;
	size_t m_size;

	static uint32_t findMemoryType(
		const vk::PhysicalDevice&, 
		uint32_t memoryTypeBits, 
		const vk::MemoryPropertyFlags&);

	BufferResource(
		const vk::UniqueDevice&,
		vk::PhysicalDevice,
		size_t,
		vk::BufferUsageFlags,
		vk::MemoryPropertyFlags);

	friend class Device;
};