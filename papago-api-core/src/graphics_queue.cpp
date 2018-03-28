#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"

void GraphicsQueue::submitCommands(std::vector<CommandBuffer> commandBuffers)
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

void GraphicsQueue::present()
{
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore };
	std::vector<vk::SwapchainKHR> swapchains = { static_cast<vk::SwapchainKHR>(m_swapChain) };

	auto imageIndices = getCurrentFrameIndex();
	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(semaphores.size())
		.setPWaitSemaphores(semaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(swapchains.data())
		.setPImageIndices(&imageIndices);

	m_vkPresentQueue.presentKHR(presentInfo);

	//TODO: handle wait another way?
	m_vkPresentQueue.waitIdle();
}

uint32_t GraphicsQueue::getCurrentFrameIndex()
{
	auto result = m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(m_swapChain), 0, *m_vkImageAvailableSemaphore, vk::Fence());
	return result.value;
}

GraphicsQueue::GraphicsQueue(const vk::UniqueDevice& device, int graphicsQueueIndex, int presentQueueIndex, SwapChain& swapChain)
	: m_vkDevice(device), m_swapChain(swapChain)
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
