#pragma once
#include <vector>
#include <set>
#include "resource.hpp"
#include "igraphics_queue.hpp"

class Device;
class CommandBuffer;
class SwapChain;
class ImageResource;
class ICommandBuffer;

class GraphicsQueue : public IGraphicsQueue
{
public:
	GraphicsQueue(const Device&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	
	void present() override;
	size_t getNextFrameIndex() override;
	void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) override;
	IImageResource& getLastRenderedImage();
	IImageResource& getLastRenderedDepthBuffer() override;
private:
	uint32_t getCurrentFrameIndex();
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
