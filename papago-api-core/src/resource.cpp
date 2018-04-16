#include "standard_header.hpp"
#include "resource.hpp"

Resource::Resource(Resource&& other) noexcept: m_vkMemory(std::move(other.m_vkMemory)), m_vkDevice(other.m_vkDevice), m_size(other.m_size)
{
	other.m_size = 0;
	other.m_vkMemory = vk::UniqueDeviceMemory();
}

void Resource::upload(const std::vector<char>& data)
{
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, VK_WHOLE_SIZE);
	memcpy(mappedMemory, data.data(), data.size());
	m_vkDevice->unmapMemory(*m_vkMemory);
}

std::vector<char> Resource::download()
{
	std::vector<char> result(m_size);
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, VK_WHOLE_SIZE);
	memcpy(result.data(), mappedMemory, m_size);
	m_vkDevice->unmapMemory(*m_vkMemory);
	return result;
}

Resource::Resource(const vk::UniqueDevice& device) : m_vkMemory(nullptr), m_vkDevice(device), m_size(0) {}

Resource::Resource(
	const vk::PhysicalDevice& physicalDevice, 
	const vk::UniqueDevice& device, 
	vk::MemoryPropertyFlags flags, 
	vk::MemoryRequirements memoryRequirements) 
		: m_vkDevice(device), 
		m_size(memoryRequirements.size)
{
	auto memoryType = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, flags);
	m_vkMemory = device->allocateMemoryUnique(
		vk::MemoryAllocateInfo()
			.setAllocationSize(memoryRequirements.size)
			.setMemoryTypeIndex(memoryType)
	);
}

uint32_t Resource::findMemoryType(
	const vk::PhysicalDevice& physicalDevice, 
	uint32_t memoryTypeBits, 
	const vk::MemoryPropertyFlags& flags)
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

	//If successful, then we have returned by now

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
	if (flags & vk::MemoryPropertyFlagBits::eLazilyAllocated)
	{
		errorStream << "LAZILY_ALLOCATED ";
	}

	PAPAGO_ERROR(errorStream.str());
}
