#pragma once
#include "standard_header.hpp"
#include <vector>
#include "api_enums.hpp"
#include "shader.hpp"
#include "command_buffer.hpp"
#include "image_resource.hpp"
//#include "surface.hpp"

class VertexShader;
class FragmentShader;
class BufferResource;
class GraphicsQueue;
class Surface;
class SwapChain;

class Device {
public:
	static Surface createSurface(size_t width, size_t height, HWND&);
	static std::vector<Device> enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions);

	SwapChain createSwapChain(Format, size_t framebufferCount, SwapChainPresentMode, Surface&);
	ImageResource createImageResource(size_t width, size_t height, TypeEnums, ImageResource::ImageType);
	BufferResource createBufferResource();
	GraphicsQueue createGraphicsQueue(SwapChain);
	CommandBuffer createCommandBuffer(CommandBuffer::Usage);
	SubCommandBuffer createSubCommandBuffer(SubCommandBuffer::Usage);
private:

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentmodes;
	};

	struct QueueFamilyIndices {
		int graphicsFamily = -1; //<-- "not found"
		int presentFamily = -1;

		bool isComplete() const
		{
			return graphicsFamily >= 0 && presentFamily >= 0;
		}
	};


	Device(vk::PhysicalDevice physicalDevice, vk::Device device) : m_VkPhysicalDevice(physicalDevice), m_VkDevice(device) {};

	static Device::QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, Surface& surface);
	SwapChainSupportDetails querySwapChainSupport(Surface& surface);
	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(Format,  std::vector<vk::SurfaceFormatKHR>& availableFormats);

	vk::PresentModeKHR chooseSwapPresentMode(SwapChainPresentMode&, const std::vector<vk::PresentModeKHR>& availablePresentModes);

	vk::Extent2D chooseSwapChainExtend(uint32_t width, uint32_t height, const vk::SurfaceCapabilitiesKHR& availableCapabilities);

	static vk::Instance s_VkInstance;
	vk::PhysicalDevice m_VkPhysicalDevice;
	vk::Device m_VkDevice;
};