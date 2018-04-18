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
class Sampler;
class ShaderProgram;

class Device {
public:
	static std::vector<Device> enumerateDevices(Surface& surface, const Features &features, const std::vector<const char*> &extensions);
	Device(vk::PhysicalDevice, vk::UniqueDevice&, Surface&);

	SwapChain createSwapChain(const Format&, size_t framebufferCount, SwapChainPresentMode);
	GraphicsQueue createGraphicsQueue(SwapChain&) const;
	CommandBuffer createCommandBuffer(Usage) const;
	SubCommandBuffer createSubCommandBuffer(Usage);
	RenderPass createRenderPass(const ShaderProgram&, uint32_t width, uint32_t height, Format, bool enableDepthBuffer) const;
	Sampler createTextureSampler3D(Filter magFil, Filter minFil, TextureWrapMode modeU, TextureWrapMode modeV, TextureWrapMode modeW);
	Sampler createTextureSampler2D(Filter magFil, Filter minFil, TextureWrapMode modeU, TextureWrapMode modeV);
	Sampler createTextureSampler1D(Filter magFil, Filter minFil, TextureWrapMode modeU);
	//drop the other create textureSampler idea below?
	void createTextureSampler(Sampler sampler);

	ImageResource createTexture2D(uint32_t width, uint32_t height, Format format);

	ShaderProgram createShaderProgram(VertexShader&, FragmentShader&);

	template<typename T>
	BufferResource createVertexBuffer(const std::vector<T>& vertexData) const;
	template<typename T>
	BufferResource createIndexBuffer(const std::vector<T>& indexData) const;
	template<size_t N>
	BufferResource createUniformBuffer() const;
	void waitIdle();
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
		
		bool hasGraphicsFamily() const {
			return graphicsFamily != NOT_FOUND();
		}
		
		bool hasPresentFamily() const {
			return presentFamily != NOT_FOUND();
		}

		bool isComplete() const
		{
			return graphicsFamily != NOT_FOUND() && presentFamily != NOT_FOUND();
		}
	};


	static SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& , Surface& ) ;
	
	static QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, Surface& surface);
	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(Format,  std::vector<vk::SurfaceFormatKHR>& availableFormats);
	static vk::PresentModeKHR chooseSwapPresentMode(SwapChainPresentMode&, const std::vector<vk::PresentModeKHR>& availablePresentModes);
	static vk::Extent2D chooseSwapChainExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& availableCapabilities);
	static std::vector<vk::DeviceQueueCreateInfo> createQueueCreateInfos(QueueFamilyIndices, const float&);
	vk::SwapchainCreateInfoKHR createSwapChainCreateInfo(Surface&, const size_t& framebufferCount, const vk::SurfaceFormatKHR&, const vk::Extent2D&, const vk::SurfaceCapabilitiesKHR&, const vk::PresentModeKHR&) const;

	static bool isPhysicalDeviceSuitable(const vk::PhysicalDevice& physicalDevice, Surface&, const std::vector<const char*> &);
	static bool areExtensionsSupported(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*> &extensions);

	vk::UniqueRenderPass createDummyRenderpass(Format, bool withDepthBuffer = true) const;

	vk::PhysicalDevice m_vkPhysicalDevice;
	vk::UniqueDevice m_vkDevice;

	Surface& m_surface;

	vk::Queue m_vkInternalQueue;
	CommandBuffer m_internalCommandBuffer;

	friend class ImageResource;
};

template<typename T>
BufferResource Device::createVertexBuffer(const std::vector<T>& vertexData) const
{
	size_t bufferSize = sizeof(T) * vertexData.size();
	auto buffer = BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
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
	auto buffer = BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eIndexBuffer,
		// TODO: Convert to device local memory when command pool and buffers are ready
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	auto data = reinterpret_cast<char const *>(indexData.data());
	//TODO: should the creation of buffers also upload right away?
	buffer.upload(std::vector<char>(data, data + bufferSize));
	return std::move(buffer);
}

template<size_t N>
BufferResource Device::createUniformBuffer() const
{
	return BufferResource::createBufferResource(
		m_vkPhysicalDevice,
		m_vkDevice,
		N,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}