#pragma once
#include "standard_header.hpp"
#include <vector>

class CommandBuffer;
class SwapChain;

class GraphicsQueue
{
public:
	void submitCommands(std::vector<CommandBuffer>&);
	void present();
	uint32_t getCurrentFrameIndex();
private:
	GraphicsQueue(const vk::UniqueDevice&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	void createSemaphores(const vk::UniqueDevice&);

	SwapChain& m_swapChain;
	vk::Queue m_vkGraphicsQueue;
	vk::Queue m_vkPresentQueue;
	vk::UniqueSemaphore m_vkRenderFinishSemaphore;
	vk::UniqueSemaphore m_vkImageAvailableSemaphore;
	const vk::UniqueDevice& m_vkDevice;

	friend class Device;
};