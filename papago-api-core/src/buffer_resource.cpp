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

bool BufferResource::inUse()
{
	
	if (m_vkFence != nullptr && m_vkDevice->getFenceStatus(*m_vkFence) == vk::Result::eNotReady) {
		return true;
	}
	else {
		return false;
	}
}


BufferResource BufferResource::createBufferResource(
	vk::PhysicalDevice		physicalDevice, 
	const vk::UniqueDevice& device, 
	size_t					size, 
	vk::BufferUsageFlags	usageFlags, 
	vk::MemoryPropertyFlags memoryFlags)
{
	auto bufferCreateInfo = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usageFlags | vk::BufferUsageFlagBits::eTransferDst); //IMPROVEMENT: All buffers are currently assumed to be able to be uploaded to

	auto vkBuffer = device->createBufferUnique(bufferCreateInfo); 

	auto memoryRequirements = device->getBufferMemoryRequirements(*vkBuffer);

	return BufferResource(device, physicalDevice, std::move(vkBuffer), memoryFlags, memoryRequirements, size);
}

BufferResource::BufferResource(
	const vk::UniqueDevice&		device,
	const vk::PhysicalDevice&	physicalDevice,
	vk::UniqueBuffer&&			buffer,
	vk::MemoryPropertyFlags		memoryFlags,
	vk::MemoryRequirements		memoryRequirements,
	size_t						range)
		: Resource(physicalDevice, device, memoryFlags, memoryRequirements)
		, m_vkBuffer(std::move(buffer))
{
	m_vkInfo.setBuffer(*m_vkBuffer)
		.setOffset(0)
		.setRange(range); 
	device->bindBufferMemory(*m_vkBuffer, *m_vkMemory, 0);
}
