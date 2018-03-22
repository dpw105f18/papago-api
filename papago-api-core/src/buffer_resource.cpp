#include "standard_header.hpp"
#include "buffer_resource.hpp"

void BufferResource::upload(std::vector<char> data)
{
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, VK_WHOLE_SIZE);
	memcpy(mappedMemory, data.data(), data.size());
	m_vkDevice->unmapMemory(*m_vkMemory);
}

void BufferResource::destroy()
{

}

std::vector<char> BufferResource::download()
{
	std::vector<char> result(m_size);
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, VK_WHOLE_SIZE);
	memcpy(result.data(), mappedMemory, m_size);
	m_vkDevice->unmapMemory(*m_vkMemory);
	return result;
}

uint32_t BufferResource::findMemoryType(
	const vk::PhysicalDevice&		physicalDevice, 
	uint32_t						memoryTypeBits, 
	const vk::MemoryPropertyFlags&	flags)
{
	auto memoryProperties = physicalDevice.getMemoryProperties();
	for (auto i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if (memoryTypeBits & 1 << i && 
			memoryProperties.memoryTypes[i].propertyFlags & flags) 
		{
			return i;
		}
	}

	std::stringstream errorStream;
	errorStream << "Failed to find memory with the properties: ";

	if (flags & vk::MemoryPropertyFlagBits::eDeviceLocal)
	{
		errorStream << "DEVICE_LOCAL ";
	}
	if (flags & vk::MemoryPropertyFlagBits::eHostVisible)
	{
		errorStream << "HOST_VISIBLE ";
	}
	if (flags & vk::MemoryPropertyFlagBits::eHostCoherent)
	{
		errorStream << "HOST_COHERENT ";
	}
	if (flags & vk::MemoryPropertyFlagBits::eHostCached)
	{
		errorStream << "HOST_CACHED ";
	}
	if(flags & vk::MemoryPropertyFlagBits::eLazilyAllocated)
	{
		errorStream << "LAZILY_ALLOCATED ";
	}

	throw std::runtime_error(errorStream.str());
}

BufferResource::BufferResource(
	const vk::UniqueDevice& device,
	vk::PhysicalDevice		physicalDevice,
	size_t					size,
	vk::BufferUsageFlags	usageFlags,
	vk::MemoryPropertyFlags memoryFlags)
	: m_vkBuffer(device->createBufferUnique(vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usageFlags | vk::BufferUsageFlagBits::eTransferDst))) // IMPROVEMENT : All buffers are currently assumed to be able to be uploaded to
	, m_vkDevice(device)
	, m_vkPhysicalDevice(physicalDevice)
	, m_size(size)
{
	auto memoryRequirements = device->getBufferMemoryRequirements(*m_vkBuffer);

	m_vkMemory = device->allocateMemoryUnique(vk::MemoryAllocateInfo()
		.setAllocationSize(size)
		.setMemoryTypeIndex(
			findMemoryType(
				physicalDevice,
				memoryRequirements.memoryTypeBits,
				memoryFlags))
	);

	device->bindBufferMemory(*m_vkBuffer, *m_vkMemory, 0);
}
