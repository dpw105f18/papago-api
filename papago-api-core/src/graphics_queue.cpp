#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"
#include <iterator>

void GraphicsQueue::submitCommands(std::vector<CommandBuffer>& commandBuffers)
{
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore };
	std::vector<vk::Semaphore> waitSemaphores = { *m_vkImageAvailableSemaphore };
	std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::vector<vk::CommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffers.size());

	for (auto& commandBuffer : commandBuffers) {
		vkCommandBuffers.emplace_back(static_cast<vk::CommandBuffer>(commandBuffer));
	}

	vk::SubmitInfo submitInfo = {};
	submitInfo.setWaitSemaphoreCount(waitSemaphores.size())
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStages.data())
		.setCommandBufferCount(vkCommandBuffers.size())
		.setPCommandBuffers(vkCommandBuffers.data())	//TODO: find out how to cast a vector<CommandBuffer> to vector<vk::CommandBuffer> -AM
		.setSignalSemaphoreCount(semaphores.size())
		.setPSignalSemaphores(semaphores.data());

	auto& fence = m_swapChain.m_vkFences[m_currentFrameIndex];
	m_vkDevice->resetFences({*fence});

	//TODO: Test if this works with several command buffers using the same resources. - Brandborg
	std::set<Resource*> resources;
	for (auto& cmd : commandBuffers) {
		std::merge(resources.begin(), resources.end(), cmd.m_resourcesInUse.begin(), cmd.m_resourcesInUse.end(), std::inserter(resources, resources.begin()));
	}

	for (auto& resource : resources) {
		resource->m_vkFence = *fence;
	}

	auto res = *resources.begin();
	auto bo = reinterpret_cast<BufferResource*>(res)->inUse();


	m_vkGraphicsQueue.submit(submitInfo, *fence);

	bo = reinterpret_cast<BufferResource*>(res)->inUse();
}

void GraphicsQueue::present(std::vector<CommandBuffer>& commandBuffers)
{
	m_vkPresentQueue.waitIdle();
	auto imageIndices = getNextFrameIndex();
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore };
	std::vector<vk::SwapchainKHR> swapchains = { static_cast<vk::SwapchainKHR>(m_swapChain) };

	submitCommands(commandBuffers);

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(semaphores.size())
		.setPWaitSemaphores(semaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(swapchains.data())
		.setPImageIndices(&imageIndices);

	auto res = m_vkPresentQueue.presentKHR(presentInfo);
}

//TODO: switch getCurrentFrameIndex and getNextFrameIndex back. -AM
uint32_t GraphicsQueue::getCurrentFrameIndex() {
	
	//TODO: Make sure that we handle the case, where framebuffer is not ready - Brandborg
	auto result = m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(m_swapChain), 0, *m_vkImageAvailableSemaphore, vk::Fence());
	m_currentFrameIndex = result.value;
	return m_currentFrameIndex;
}

//TODO: don't present this to API users. -AM
void GraphicsQueue::Wait()
{
	m_vkPresentQueue.waitIdle();
}

uint32_t GraphicsQueue::getNextFrameIndex()
{
	return m_currentFrameIndex;
}

GraphicsQueue::GraphicsQueue(const vk::UniqueDevice& device, int graphicsQueueIndex, int presentQueueIndex, SwapChain& swapChain)
	: m_swapChain(swapChain), m_vkDevice(device)
{
	m_vkGraphicsQueue = device->getQueue(graphicsQueueIndex, 0);
	m_vkPresentQueue = device->getQueue(presentQueueIndex, 0);

	createSemaphores(device);
}

void GraphicsQueue::createSemaphores(const vk::UniqueDevice &device)
{
	m_vkRenderFinishSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
	m_vkImageAvailableSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
}
