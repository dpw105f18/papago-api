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
	uint32_t getNextFrameIndex();
	void wait();
	void submitCommands(std::vector<CommandBuffer>&);
	ImageResource& getLastRenderedImage();
private:
	uint32_t getCurrentFrameIndex();
	GraphicsQueue(const Device&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	void createSemaphores(const vk::UniqueDevice&);

	template<vk::ImageLayout from, vk::ImageLayout to>
	static void transitionImageResources(const CommandBuffer&, const vk::Queue&, std::set<ImageResource*> resources);

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

template<vk::ImageLayout from, vk::ImageLayout to>
inline void GraphicsQueue::transitionImageResources(const CommandBuffer& commandBuffer, const vk::Queue& queue, std::set<ImageResource*> resources) {
	auto commandBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	commandBuffer->begin(commandBeginInfo);

	//Transition submitted ImageResources to ePresentSrcKHR:
	for (auto& resource : resources) {
		resource->transition<from, to>(commandBuffer);
	}

	commandBuffer->end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*commandBuffer);
	queue.submit(submitInfo, vk::Fence());

	queue.waitIdle();

	commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);
}
