#pragma once
#include <vector>
#include <set>
#include "resource.hpp"

class Device;
class CommandBuffer;
class SwapChain;
class ImageResource;

class GraphicsQueue
{
public:
	void present();
	uint32_t getCurrentFrameIndex();
	void wait();
	void submitCommands(std::vector<CommandBuffer>&);
	ImageResource& getLastRenderedImage();
private:
	uint32_t getNextFrameIndex();
	GraphicsQueue(const Device&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	void createSemaphores(const vk::UniqueDevice&);

	SwapChain& m_swapChain;
	vk::Queue m_vkGraphicsQueue;
	vk::Queue m_vkPresentQueue;
	vk::UniqueSemaphore m_vkRenderFinishSemaphore;
	vk::UniqueSemaphore m_vkImageAvailableSemaphore;
	const Device& m_device;
	uint32_t m_currentFrameIndex = 0; // ASSUMPTION: Start index is always 0

	std::set<Resource*> m_submittedResources;

	friend class Device;
};
