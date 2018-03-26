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
	BufferResource createBufferResource();
	GraphicsQueue createGraphicsQueue(SwapChain);
	CommandBuffer createCommandBuffer(CommandBuffer::Usage);
	SubCommandBuffer createSubCommandBuffer(SubCommandBuffer::Usage);
	VertexShader createVertexShader(const std::string& filePath, const std::string& entryPoint);
	FragmentShader createFragmentShader(const std::string& filePath, const std::string& entryPoint);
	RenderPass createRenderPass(VertexShader&, FragmentShader&, const SwapChain&);


	template<typename T>
	BufferResource createVertexBuffer(const std::vector<T>& vertexData) const;
	template<typename T>
	BufferResource createIndexBuffer(const std::vector<T>& indexData) const;
	template<size_t N>
	BufferResource createUniformBuffer() const;
private:
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
BufferResource Device::createVertexBuffer(const std::vector<T>& vertexData) const
{
	size_t bufferSize = sizeof(T) * vertexData.size();
	auto buffer = BufferResource(
		m_vkDevice,
		m_vkPhysicalDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		// TODO: Convert to device local memory when command pool and buffers are ready
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto data = reinterpret_cast<char const *>(vertexData.data());
	buffer.upload(std::vector<char>(data, data + bufferSize));
	return buffer;
}

template<typename T>
BufferResource Device::createIndexBuffer(const std::vector<T>& indexData) const
{
	size_t bufferSize = sizeof(T) * indexData.size();
	auto buffer = BufferResource(
		m_vkDevice,
		m_vkPhysicalDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eIndexBuffer,
		// TODO: Convert to device local memory when command pool and buffers are ready
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto data = reinterpret_cast<char const *>(indexData.data());
	buffer.upload(std::vector<char>(data, data + bufferSize));
	return buffer;
}

template<size_t N>
BufferResource Device::createUniformBuffer() const
{
	return BufferResource(
		m_vkDevice,
		m_vkPhysicalDevice,
		N,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}
