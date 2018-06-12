#include "standard_header.hpp"
#include "buffer_resource.hpp"

BufferResource::BufferResource(BufferResource&& other) noexcept
	: Resource(std::move(other))
	, m_vkBuffer(std::move(other.m_vkBuffer))
	, m_elementType(other.m_elementType)
{
}

void BufferResource::upload(const std::vector<char>& data, size_t offset)
{
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, offset, data.size());
	memcpy(mappedMemory, data.data(), data.size());
	m_vkDevice->unmapMemory(*m_vkMemory);
}

inline bool BufferResource::inUse()
{
	return m_vkFence
		&& m_vkDevice->getFenceStatus(*m_vkFence) == vk::Result::eNotReady;
}

void BufferResource::internalUpload(const std::vector<char>& data)
{
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, data.size());
	memcpy(mappedMemory, data.data(), data.size());
	m_vkDevice->unmapMemory(*m_vkMemory);
}

std::vector<char> BufferResource::internalDownload()
{
	std::vector<char> result(m_size);
	auto mappedMemory = m_vkDevice->mapMemory(*m_vkMemory, 0, VK_WHOLE_SIZE);
	memcpy(result.data(), mappedMemory, m_size);
	m_vkDevice->unmapMemory(*m_vkMemory);
	return result;
}

std::unique_ptr<BufferResource> BufferResource::createBufferResource(
	vk::PhysicalDevice			physicalDevice, 
	const vk::UniqueDevice&		device, 
	size_t						size, 
	vk::BufferUsageFlags		usageFlags, 
	vk::MemoryPropertyFlags		memoryFlags,
	BufferResourceElementType	type)
{
	auto bufferCreateInfo = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usageFlags | vk::BufferUsageFlagBits::eTransferDst); //IMPROVEMENT: All buffers are currently assumed to be able to be uploaded to

	auto vkBuffer = device->createBufferUnique(bufferCreateInfo); 

	auto memoryRequirements = device->getBufferMemoryRequirements(*vkBuffer);

	return std::make_unique<BufferResource>(device, physicalDevice, std::move(vkBuffer), memoryFlags, memoryRequirements, size);
}

BufferResource::BufferResource(
	const vk::UniqueDevice&				device,
	const vk::PhysicalDevice&			physicalDevice,
	vk::UniqueBuffer&&					buffer,
	vk::MemoryPropertyFlags				memoryFlags,
	vk::MemoryRequirements				memoryRequirements,
	size_t								range,
	const BufferResourceElementType		type)
		: Resource(physicalDevice, device, memoryFlags, memoryRequirements)
		, m_vkBuffer(std::move(buffer))
		, m_elementType(type)
{
	m_vkInfo.setBuffer(*m_vkBuffer)
		.setOffset(0)
		.setRange(range); 
	device->bindBufferMemory(*m_vkBuffer, *m_vkMemory, 0);
}


std::vector<char> DynamicBufferResource::internalDownload()
{
	return m_buffer->download();
}

void DynamicBufferResource::internalUpload(const std::vector<char>& data, size_t offset)
{
	auto& internalBuffer = dynamic_cast<BufferResource&>(*m_buffer);
	internalBuffer.upload(data, offset);
}

size_t DynamicBufferResource::getAlignment()
{
	return m_alignment;
}

void DynamicBufferResource::uploadPadded(const std::vector<char>& data)
{
	internalUpload(data);
}
