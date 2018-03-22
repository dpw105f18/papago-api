#pragma once
#include "resource.hpp"
#include "device.hpp"

class BufferResource : public Resource
{
public:
	// Inherited via Resource
	void upload(const std::vector<char>& data) override;
	void destroy() override;
	std::vector<char> download() override;

	size_t getSize() const { return m_size; }

private:
	vk::UniqueBuffer m_vkBuffer;
	vk::UniqueDeviceMemory m_vkMemory;
	const vk::UniqueDevice& m_vkDevice;
	size_t m_size;

	static uint32_t findMemoryType(
		const vk::PhysicalDevice&, 
		uint32_t memoryTypeBits, 
		const vk::MemoryPropertyFlags&);

	BufferResource(
		const vk::UniqueDevice&,
		const vk::PhysicalDevice&,
		size_t,
		vk::BufferUsageFlags,
		vk::MemoryPropertyFlags);

	friend class Device;
};