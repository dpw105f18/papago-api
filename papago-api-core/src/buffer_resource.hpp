#pragma once
#include "vulkan\vulkan.hpp"
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
		vk::MemoryRequirements		memoryRequirements,
		size_t						range,
		BufferResourceElementType	type = BufferResourceElementType::eChar);
	BufferResource(const BufferResource&) = delete;
	BufferResource(BufferResource&& other) noexcept;

	// Inherited via Resource
	bool inUse() override;
	const BufferResourceElementType m_elementType;
	vk::UniqueBuffer m_vkBuffer;
	vk::DescriptorBufferInfo m_vkInfo;

	static std::unique_ptr<BufferResource> createBufferResource(
		vk::PhysicalDevice			physicalDevice,
		const vk::UniqueDevice&		device,
		size_t						size,
		vk::BufferUsageFlags		usageFlags,
		vk::MemoryPropertyFlags		memoryFlags,
		BufferResourceElementType	type = BufferResourceElementType::eChar);

protected:
	void internalUpload(const std::vector<char>& data) override;
	std::vector<char> internalDownload() override;
private:

};

class DynamicBuffer 
	: public IDynamicBuffer 
{
public:
	std::unique_ptr<IBufferResource> m_buffer;
	size_t m_alignment;
	size_t m_objectCount;

	DynamicBuffer(
		std::unique_ptr<IBufferResource>&&	buffer,
		size_t								alignment,
		size_t								objectCount);

	IBufferResource& innerBuffer() { return *m_buffer; }

	// Inherited via IDynamicBuffer
	void internalUpload(const std::vector<char>& data) override;
	std::vector<char> internalDownload() override;
};

//TODO: move to buffer_resource.cpp?
inline DynamicBuffer::DynamicBuffer(
	std::unique_ptr<IBufferResource>&& buffer,
	const size_t                       alignment,
	const size_t						objectCount)
	: m_buffer(std::move(buffer))
	, m_alignment(alignment)
	, m_objectCount(objectCount)
{ }
