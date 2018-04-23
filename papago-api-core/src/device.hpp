#pragma once
#include <vector>
#include "IDevice.hpp"
#include "api_enums.hpp"
#include "command_buffer.hpp"
#include "buffer_resource.hpp"

class IVertexShader;
class IFragmentShader;
class BufferResource;
class GraphicsQueue;
class Surface;
class SwapChain;
class ImageResource;
class Sampler;
class ShaderProgram;

class Device : public IDevice {
public:
	static std::vector<Device> enumerateDevices(Surface& surface, const Features &features, const std::vector<const char*> &extensions);
	Device(vk::PhysicalDevice, vk::UniqueDevice&, Surface&);


	std::unique_ptr<ISwapchain> createSwapChain(Format, size_t framebufferCount, PresentMode preferredPresentMode) override;
	std::unique_ptr<SwapChain> createSwapChain(const vk::Format & format, size_t framebufferCount, vk::PresentModeKHR preferredPresentMode);
	GraphicsQueue createGraphicsQueue(SwapChain&) const;
	CommandBuffer createCommandBuffer(Usage) const;
	SubCommandBuffer createSubCommandBuffer(Usage);
	RenderPass createRenderPass(const ShaderProgram&, uint32_t width, uint32_t height, Format, bool enableDepthBuffer) const;
	std::unique_ptr<ISampler> createTextureSampler1D(Filter magFil, Filter minFil, TextureWrapMode modeU);
	std::unique_ptr<ISampler> createTextureSampler2D(Filter magFil, Filter minFil, TextureWrapMode modeU, TextureWrapMode modeV);
	std::unique_ptr<ISampler> createTextureSampler3D(Filter magFil, Filter minFil, TextureWrapMode modeU, TextureWrapMode modeV, TextureWrapMode modeW);
	std::unique_ptr<IImageResource> createTexture2D(size_t width, size_t height, Format) override;
	//drop the other create textureSampler idea below?
	void createTextureSampler(Sampler sampler);

	ImageResource createTexture2D(uint32_t width, uint32_t height, vk::Format format);

	ShaderProgram createShaderProgram(IVertexShader&, IFragmentShader&);
	std::unique_ptr<IBufferResource> createUniformBuffer(size_t size) override;

	void waitIdle();
protected:
	std::unique_ptr<IBufferResource> createVertexBufferInternal(std::vector<char>& data) override;
	std::unique_ptr<IBufferResource> createIndexBufferInternal(std::vector<char>& data) override;
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
	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(vk::Format,  std::vector<vk::SurfaceFormatKHR>& availableFormats);
	static vk::PresentModeKHR chooseSwapPresentMode(vk::PresentModeKHR, const std::vector<vk::PresentModeKHR>& availablePresentModes);
	static vk::Extent2D chooseSwapChainExtent(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& availableCapabilities);
	static std::vector<vk::DeviceQueueCreateInfo> createQueueCreateInfos(QueueFamilyIndices, const float&);
	vk::SwapchainCreateInfoKHR createSwapChainCreateInfo(Surface&, const size_t& framebufferCount, const vk::SurfaceFormatKHR&, const vk::Extent2D&, const vk::SurfaceCapabilitiesKHR&, const vk::PresentModeKHR&) const;

	static bool isPhysicalDeviceSuitable(const vk::PhysicalDevice& physicalDevice, Surface&, const std::vector<const char*> &);
	static bool areExtensionsSupported(const vk::PhysicalDevice& physicalDevice, const std::vector<const char*> &extensions);

	vk::UniqueRenderPass createVkRenderpass(Format, bool withDepthBuffer = true) const;

	vk::PhysicalDevice m_vkPhysicalDevice;
	vk::UniqueDevice m_vkDevice;

	Surface& m_surface;

	vk::Queue m_vkInternalQueue;
	CommandBuffer m_internalCommandBuffer;

	friend class ImageResource;
	friend class GraphicsQueue;
};