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
