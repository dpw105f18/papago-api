#include <iterator>
#include "standard_header.hpp"
#include "graphics_queue.hpp"
#include "swap_chain.hpp"
#include "device.hpp"
#include "ibuffer_resource.hpp"
#include "image_resource.hpp"
#include "image_resource.impl"

void GraphicsQueue::submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>& commandBuffers)
{
	//m_vkGraphicsQueue.waitIdle();
	std::vector<vk::Semaphore> semaphores = { *m_vkRenderFinishSemaphore};
	std::vector<vk::CommandBuffer> vkCommandBuffers;
	vkCommandBuffers.reserve(commandBuffers.size());

	for (auto& commandBuffer : commandBuffers) {
		vkCommandBuffers.emplace_back(static_cast<vk::CommandBuffer>(static_cast<CommandBuffer&>(commandBuffer.get())));
	}

	vk::SubmitInfo submitInfo = {};
	submitInfo.setCommandBufferCount(vkCommandBuffers.size())
		.setPCommandBuffers(vkCommandBuffers.data())
		.setSignalSemaphoreCount(semaphores.size())
		.setPSignalSemaphores(semaphores.data());

	int fenceIndex = -1;
	for (auto i = 0; i < m_vkFences.size(); ++i) {
		if (m_device.m_vkDevice->getFenceStatus(*m_vkFences[i]) == vk::Result::eSuccess) {
			fenceIndex = i;
			break;
		}
	}

	if (fenceIndex == -1) {
		fenceIndex = m_vkFences.size();
		m_vkFences.emplace_back(std::move(m_device.m_vkDevice->createFenceUnique({})));
	}

	auto& fence = m_vkFences[fenceIndex];
	m_device.m_vkDevice->resetFences({*fence});	//<-- possible bug: newly created fences might not like to be reset.

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

void GraphicsQueue::present(ISwapchain& swapchain)
{
	auto& internalSwapChain = dynamic_cast<SwapChain&>(swapchain);
	//m_vkPresentQueue.waitIdle();

	std::set<ImageResource*> imageResources;
	imageResources.emplace(&internalSwapChain.m_colorResources[internalSwapChain.m_currentFramebufferIndex]);

	// Find image resources from the submitted resources 
	for (auto& resource : m_submittedResources) {
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
	std::vector<vk::SwapchainKHR> swapchains = { static_cast<vk::SwapchainKHR>(internalSwapChain) };

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.setWaitSemaphoreCount(semaphores.size())
		.setPWaitSemaphores(semaphores.data())
		.setSwapchainCount(1)
		.setPSwapchains(swapchains.data())
		.setPImageIndices(&internalSwapChain.m_currentFramebufferIndex);

	auto res = m_vkPresentQueue.presentKHR(presentInfo);

	//TODO: find a better way to know when ImageResources can safely be transitioned back to eGeneral layout, Fences??
	m_vkPresentQueue.waitIdle();

	transitionImageResources<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(
		m_device.m_internalCommandBuffer,
		m_device.m_vkInternalQueue,
		imageResources
		);

	m_submittedResources.clear();
	//TODO: find some way to not create new fences every present.


	vk::UniqueFence fence = m_device.m_vkDevice->createFenceUnique({});
	auto nextFramebuffer = m_device.m_vkDevice->acquireNextImageKHR(static_cast<vk::SwapchainKHR>(internalSwapChain), 0, {}, *fence).value;

	auto& oldFence = internalSwapChain.m_vkFences[nextFramebuffer];

	
	if(*oldFence != vk::Fence() && m_device.m_vkDevice->getFenceStatus(*oldFence) != vk::Result::eSuccess) {
		m_vkPresentQueue.waitIdle();
	}

	oldFence = std::move(fence);
	internalSwapChain.m_currentFramebufferIndex = nextFramebuffer;
}


GraphicsQueue::GraphicsQueue(const Device& device, int graphicsQueueIndex, int presentQueueIndex)
	: m_device(device)
{
	m_vkGraphicsQueue = device.m_vkDevice->getQueue(graphicsQueueIndex, 0);
	m_vkPresentQueue = device.m_vkDevice->getQueue(presentQueueIndex, 0);

	createSemaphores(device.m_vkDevice);
}

void GraphicsQueue::createSemaphores(const vk::UniqueDevice &device)
{
	m_vkRenderFinishSemaphore = device->createSemaphoreUnique(vk::SemaphoreCreateInfo());
}

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
