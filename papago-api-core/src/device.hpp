#pragma once
#include "standard_header.hpp"
#include <vector>
#include "api_enums.hpp"
#include "command_buffer.hpp"
#include "buffer_resource.hpp"

class VertexShader;
class FragmentShader;
class BufferResource;
class GraphicsQueue;
class Surface;
class SwapChain;
class ImageResource;

class Device {
public:
	static std::vector<Device> enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions);

	SwapChain createSwapChain(const Format&, size_t framebufferCount, SwapChainPresentMode, Surface&);
	ImageResource createImageResource(size_t width, size_t height, TypeEnums, ImageType);	//<-- TODO: pick up here! [Texture Resource story]
	BufferResource createBufferResource();
	GraphicsQueue createGraphicsQueue(SwapChain);
	CommandBuffer createCommandBuffer(CommandBuffer::Usage);
	SubCommandBuffer createSubCommandBuffer(SubCommandBuffer::Usage);

	template<typename T>
	BufferResource createVertexBuffer(const std::vector<T>& vertexData);
private:
	BufferResource createStagingBuffer(size_t size);

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentmodes;
	};

	struct QueueFamilyIndices {
		static constexpr int NOT_FOUND() { return -1; }

		int graphicsFamily = NOT_FOUND();
		int presentFamily = NOT_FOUND();

		bool isComplete() const
		{
			return graphicsFamily != NOT_FOUND() && presentFamily != NOT_FOUND();
		}
	};

	Device(vk::PhysicalDevice, vk::UniqueDevice&);

	SwapChainSupportDetails querySwapChainSupport(Surface&) const;
	
	static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, Surface& surface);
	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(Format,  std::vector<vk::SurfaceFormatKHR>& availableFormats);
	static vk::PresentModeKHR chooseSwapPresentMode(SwapChainPresentMode&, const std::vector<vk::PresentModeKHR>& availablePresentModes);
	static vk::Extent2D chooseSwapChainExtend(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& availableCapabilities);

	vk::PhysicalDevice m_vkPhysicalDevice;
	vk::UniqueDevice m_vkDevice;
};

template<typename T>
BufferResource Device::createVertexBuffer(const Device& device, const std::vector<T>& vertexData)
{
	auto bufferSize = sizeof(T) * vertexData.size();

	auto stagingBuffer = createStagingBuffer(bufferSize);

	memcpy(stagingBuffer.map(), s_Vertices.data(), bufferSize);
	stagingBuffer.unmap();

	buffer_create_info.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;

	auto vertexBuffer = std::make_unique<Buffer>(m_PhysicalDevice, m_LogicalDevice, buffer_create_info, vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(stagingBuffer.m_Buffer, vertexBuffer->m_Buffer, bufferSize);
}
