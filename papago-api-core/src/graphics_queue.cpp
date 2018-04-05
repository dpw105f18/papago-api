#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"

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

	m_vkGraphicsQueue.submit(submitInfo, vk::Fence());
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

	m_vkPresentQueue.presentKHR(presentInfo);
}

uint32_t GraphicsQueue::getCurrentFrameIndex() const {
	return m_currentFrameIndex;
}

uint32_t GraphicsQueue::getNextFrameIndex()
{
	auto result = m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(m_swapChain), 0, *m_vkImageAvailableSemaphore, vk::Fence());
	m_currentFrameIndex = result.value;
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
