#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"
#include "device.hpp"
#include <iterator>
#include <typeinfo>

void GraphicsQueue::submitCommands(std::vector<CommandBuffer>& commandBuffers)
{
	m_vkGraphicsQueue.waitIdle();
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore};
	std::vector<vk::Semaphore> waitSemaphores = { *m_vkImageAvailableSemaphore};
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
	m_device.m_vkDevice->resetFences({*fence});

	//TODO: Test if this works with several command buffers using the same resources. - Brandborg
	for (auto& cmd : commandBuffers) {
		std::merge(m_submittedResources.begin(), m_submittedResources.end(), cmd.m_resourcesInUse.begin(), cmd.m_resourcesInUse.end(), std::inserter(m_submittedResources, m_submittedResources.begin()));
		cmd.m_resourcesInUse.clear();
	}

	for (auto& resource : m_submittedResources) {
		resource->m_vkFence = &fence.get();
	}

	m_vkGraphicsQueue.submit(submitInfo, *fence);

}

ImageResource & GraphicsQueue::getLastRenderedImage()
{
	return m_swapChain.m_colorResources[m_currentFrameIndex];
}

void GraphicsQueue::present()
{
	m_vkPresentQueue.waitIdle();

	auto imageIndices = getNextFrameIndex();
	
	//Transition Framebuffer images to ePresentSrcKHR:
	auto& commandBuffer = m_device.m_internalCommandBuffer;

	auto commandBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	commandBuffer->begin(commandBeginInfo);

	m_swapChain.m_colorResources[imageIndices].transition<vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR>(commandBuffer);

	//Transition submitted ImageResources to ePresentSrcKHR:
	if (m_submittedResources.size() > 0) {
		for (auto& resource : m_submittedResources) {
			//TODO: separate the resourcesInUse on CommandBuffer into Buffer and Image iso this check? -AM
			if (typeid(*resource) == typeid(ImageResource)) {
				auto imageResource = reinterpret_cast<ImageResource*>(resource);

				imageResource->transition<vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR>(commandBuffer);
			}
		}
	}

	commandBuffer->end();

	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(1)
		.setPCommandBuffers(&*commandBuffer);
	m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());

	m_device.m_vkInternalQueue.waitIdle();

	commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore };
	std::vector<vk::SwapchainKHR> swapchains = { static_cast<vk::SwapchainKHR>(m_swapChain) };

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(semaphores.size())
		.setPWaitSemaphores(semaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(swapchains.data())
		.setPImageIndices(&imageIndices);

	auto res = m_vkPresentQueue.presentKHR(presentInfo);

	//TODO: find a better way to know when ImageResources can safely be transitioned back to eGeneral layout
	m_vkPresentQueue.waitIdle();

	//Transition Frambuffer images and ImageResources back to eGeneral:
	commandBuffer->begin(commandBeginInfo);
	m_swapChain.m_colorResources[imageIndices].transition<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(commandBuffer);

	if (m_submittedResources.size() > 0) {

		for (auto& resource : m_submittedResources) {
			//TODO: separate the resourcesInUse on CommandBuffer into Buffer and Image iso this check? -AM
			if (typeid(*resource) == typeid(ImageResource)) {
				auto imageResource = reinterpret_cast<ImageResource*>(resource);
				imageResource->transition<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(commandBuffer);
			}
		}

		commandBuffer->end();
		m_device.m_vkInternalQueue.submit(submitInfo, vk::Fence());
		m_device.m_vkInternalQueue.waitIdle();
		commandBuffer->reset(vk::CommandBufferResetFlagBits::eReleaseResources);

	}
}

//TODO: switch getCurrentFrameIndex and getNextFrameIndex back. -AM
uint32_t GraphicsQueue::getCurrentFrameIndex() {
	
	//TODO: Make sure that we handle the case, where framebuffer is not ready - Brandborg
	auto result = m_device.m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(m_swapChain), 0, *m_vkImageAvailableSemaphore, vk::Fence());
	m_currentFrameIndex = result.value;
	return m_currentFrameIndex;
}

//TODO: don't present this to API users. -AM
void GraphicsQueue::wait()
{
	m_vkPresentQueue.waitIdle();
}

uint32_t GraphicsQueue::getNextFrameIndex()
{
	return m_currentFrameIndex;
}

GraphicsQueue::GraphicsQueue(const Device& device, int graphicsQueueIndex, int presentQueueIndex, SwapChain& swapChain)
	: m_swapChain(swapChain), m_device(device)
{
	m_vkGraphicsQueue = device.m_vkDevice->getQueue(graphicsQueueIndex, 0);
	m_vkPresentQueue = device.m_vkDevice->getQueue(presentQueueIndex, 0);

	createSemaphores(device.m_vkDevice);
}

void GraphicsQueue::createSemaphores(const vk::UniqueDevice &device)
{
	m_vkRenderFinishSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
	m_vkImageAvailableSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
}
