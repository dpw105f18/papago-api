#pragma once
#include "standard_header.hpp"
#include <vector>

class CommandBuffer;
class SwapChain;

class GraphicsQueue
{
public:
	void present(std::vector<CommandBuffer>&);
	uint32_t getCurrentFrameIndex() const;
private:
	uint32_t getNextFrameIndex();
	void submitCommands(std::vector<CommandBuffer>&);
	GraphicsQueue(const vk::UniqueDevice&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	void createSemaphores(const vk::UniqueDevice&);

	SwapChain& m_swapChain;
	vk::Queue m_vkGraphicsQueue;
	vk::Queue m_vkPresentQueue;
	vk::UniqueSemaphore m_vkRenderFinishSemaphore;
	vk::UniqueSemaphore m_vkImageAvailableSemaphore;
	const vk::UniqueDevice& m_vkDevice;
	uint32_t m_currentFrameIndex = 0; // ASSUMPTION: Start index is always 0

	friend class Device;
};