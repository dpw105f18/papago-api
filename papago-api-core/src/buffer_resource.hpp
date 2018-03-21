#pragma once
#include "resource.hpp"
#include "device.hpp"

class BufferResource : public Resource
{
public:
	template<typename T>
	static BufferResource createVertexBuffer(const Device& device, const std::vector<T>& vertexData)
	{
		auto bufferSize = sizeof(T)*vertexData.size();

		auto stagingBuffer = createStagingBuffer(bufferSize);

		memcpy(stagingBuffer.map(), s_Vertices.data(), bufferSize);
		stagingBuffer.unmap();

		buffer_create_info.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

		auto vertexBuffer = std::make_unique<Buffer>(m_PhysicalDevice, m_LogicalDevice, buffer_create_info, vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer(stagingBuffer.m_Buffer, vertexBuffer->m_Buffer, bufferSize);
	}

	template<typename T>
	static BufferResource createIndexBuffer(size_t size)
	{

	}

	template<typename T>
	static BufferResource createUniformBuffer()
	{

	}

private:
	static BufferResource createStagingBuffer(const Device& device, size_t size)
	{
		return BufferResource(
			device,
			vk::BufferCreateInfo()
			.setSize(bufferSize)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	}

	BufferResource()
	{

	}

	// Inherited via Resource
	void upload(std::vector<char> data) override;
	void destroy() override;
	void download() override;
};