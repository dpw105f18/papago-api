#pragma once
#include "standard_header.hpp"
#include <vector>
#include "api_enums.hpp"
#include "shader.hpp"
#include "command_buffer.hpp"
#include "image_resource.hpp"


class SwapChain;
class VertexShader;
class FragmentShader;
class BufferResource;
class GraphicsQueue;
class Surface;

class Device {
public:
	static Surface createSurface(HWND&);
	static std::vector<Device> enumerateDevices(Surface& surface, const vk::PhysicalDeviceFeatures &features, const std::vector<const char*> &extensions);

	SwapChain createSwapChain(VertexShader, FragmentShader);
	ImageResource createImageResource(size_t width, size_t height, TypeEnums, ImageResource::ImageType);
	BufferResource createBufferResource();
	GraphicsQueue createGraphicsQueue(SwapChain);
	CommandBuffer createCommandBuffer(CommandBuffer::Usage);
	SubCommandBuffer createSubCommandBuffer(SubCommandBuffer::Usage);
private:
	Device(vk::PhysicalDevice physicalDevice, vk::Device device) : m_VkPhysicalDevice(physicalDevice), m_VkDevice(device) {};
	static vk::Instance m_VkInstance;
	vk::PhysicalDevice m_VkPhysicalDevice;
	vk::Device m_VkDevice;
};