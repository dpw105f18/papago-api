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

class Device {
public:
	static std::vector<Device> enumerateDevices();
	SwapChain createSwapChain(VertexShader, FragmentShader);
	ImageResource createImageResource(size_t width, size_t height, TypeEnums, ImageResource::ImageType);
	BufferResource createBufferResource();
	GraphicsQueue createGraphicsQueue(SwapChain);
	CommandBuffer createCommandBuffer(CommandBuffer::Usage);
	SubCommandBuffer createSubCommandBuffer(SubCommandBuffer::Usage);
private:
	static vk::Instance m_VkInstance;
};