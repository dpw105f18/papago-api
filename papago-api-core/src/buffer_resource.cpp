#include "standard_header.hpp"
#include "buffer_resource.hpp"

BufferResource::BufferResource(BufferResource&& other) noexcept
	: Resource(std::move(other))
	, m_vkBuffer(std::move(other.m_vkBuffer))
{
}

void BufferResource::destroy()
{

}

BufferResource BufferResource::createBufferResource(
	vk::PhysicalDevice		physicalDevice, 
	const vk::UniqueDevice& device, 
	size_t					size, 
	vk::BufferUsageFlags	usageFlags, 
	vk::MemoryPropertyFlags memoryFlags)
{
	auto vkBuffer(device->createBufferUnique(vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usageFlags | vk::BufferUsageFlagBits::eTransferDst))); // IMPROVEMENT : All buffers are currently assumed to be able to be uploaded to

	auto memoryRequirements = device->getBufferMemoryRequirements(*vkBuffer);

	return BufferResource(device, physicalDevice, std::move(vkBuffer), memoryFlags, memoryRequirements);
}

BufferResource::BufferResource(
	const vk::UniqueDevice&		device,
	const vk::PhysicalDevice&	physicalDevice,
	vk::UniqueBuffer&&			buffer,
	vk::MemoryPropertyFlags		memoryFlags,
	vk::MemoryRequirements		memoryRequirements)
	: Resource(physicalDevice, device, memoryFlags, memoryRequirements)
	, m_vkBuffer(std::move(buffer))
{
	device->bindBufferMemory(*m_vkBuffer, *m_vkMemory, 0);
}
