#pragma once
#include "resource.hpp"
#include "ibuffer_resource.hpp"
#include "device.hpp"

class BufferResource : public Resource, public IBufferResource
{
public:
	BufferResource(
		const vk::UniqueDevice&		device,
		const vk::PhysicalDevice&	physicalDevice,
		vk::UniqueBuffer&&			buffer,
		vk::MemoryPropertyFlags		memoryFlags,
		vk::MemoryRequirements		memoryRequirements);
	BufferResource(const BufferResource&) = delete;
	BufferResource(BufferResource&& other) noexcept;

	// Inherited via Resource
	void destroy() override;

private:
	vk::UniqueBuffer m_vkBuffer;
	vk::DescriptorBufferInfo m_vkInfo;

	static std::unique_ptr<BufferResource> createBufferResource(
		vk::PhysicalDevice		physicalDevice,
		const vk::UniqueDevice& device,
		size_t					size,
		vk::BufferUsageFlags	usageFlags,
		vk::MemoryPropertyFlags memoryFlags);

	BufferResource(
		const vk::UniqueDevice&		device,
		const vk::PhysicalDevice&	physicalDevice,
		vk::UniqueBuffer&&			buffer,
		vk::MemoryPropertyFlags		memoryFlags,
		vk::MemoryRequirements		memoryRequirements,
		size_t						range);

	friend class Device;
	friend class CommandBuffer;
	friend class GraphicsQueue;
};
