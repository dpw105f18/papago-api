#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"
#include "device.hpp"
#include <iterator>

void GraphicsQueue::submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>& commandBuffers)
{
	m_vkGraphicsQueue.waitIdle();
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore};
	std::vector<vk::Semaphore> waitSemaphores = { *m_vkImageAvailableSemaphore};
	std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::vector<vk::CommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffers.size());

	for (auto& commandBuffer : commandBuffers) {
		vkCommandBuffers.emplace_back(static_cast<vk::CommandBuffer>((CommandBuffer&)commandBuffer.get()));
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
		CommandBuffer& commandBuffer = (CommandBuffer&)cmd.get();
		std::merge(																	// Merge ..
			ITERATE(m_submittedResources),											// .. this ..
			ITERATE(commandBuffer.m_resourcesInUse),								// .. and this ..
			std::inserter(m_submittedResources, m_submittedResources.begin()));		// .. into that
		commandBuffer.m_resourcesInUse.clear();
	}

	for (auto& resource : m_submittedResources) {
		resource->m_vkFence = &(*fence);
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

	auto imageIndex = getCurrentFrameIndex();
	
	std::set<ImageResource*> imageResources;
	imageResources.emplace(&m_swapChain.m_colorResources[imageIndex]);

	// Find image resources from the submitted resources 
	for (auto& resource : m_submittedResources) {
		//TODO: separate the resourcesInUse on CommandBuffer into Buffer and Image iso this check? -AM
		if (typeid(*resource) == typeid(ImageResource)) {
			auto imageResource = reinterpret_cast<ImageResource*>(resource);
			imageResources.emplace(imageResource);
		}
	}

	transitionImageResources<vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR>(
		m_device.m_internalCommandBuffer, 
		m_device.m_vkInternalQueue, 
		imageResources
	);

	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore };
	std::vector<vk::SwapchainKHR> swapchains = { static_cast<vk::SwapchainKHR>(m_swapChain) };

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(semaphores.size())
		.setPWaitSemaphores(semaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(swapchains.data())
		.setPImageIndices(&imageIndex);

	auto res = m_vkPresentQueue.presentKHR(presentInfo);

	//TODO: find a better way to know when ImageResources can safely be transitioned back to eGeneral layout, Fences??
	m_vkPresentQueue.waitIdle();

	transitionImageResources<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(
		m_device.m_internalCommandBuffer,
		m_device.m_vkInternalQueue,
		imageResources
	);
}

//TODO: switch getCurrentFrameIndex and getNextFrameIndex back. -AM
size_t GraphicsQueue::getNextFrameIndex() {
	
	//TODO: Make sure that we handle the case, where framebuffer is not ready - Brandborg
	auto result = m_device.m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(m_swapChain), 0, *m_vkImageAvailableSemaphore, vk::Fence());
	m_currentFrameIndex = result.value;
	return m_currentFrameIndex;
}

//TODO: don't present this to API users. -AM
void GraphicsQueue::wait()
{
	m_vkGraphicsQueue.waitIdle();
	m_vkPresentQueue.waitIdle();
}

uint32_t GraphicsQueue::getCurrentFrameIndex()
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
