#pragma once
#include <vector>
#include <set>
#include "resource.hpp"
#include "igraphics_queue.hpp"

class Device;
class CommandBuffer;
class SwapChain;
class ICommandBuffer;
class ImageResource;

class GraphicsQueue : public IGraphicsQueue
{
public:
	GraphicsQueue(const Device&, int graphicsQueueIndex, int presentQueueIndex, SwapChain&);
	
	void present() override;
	void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) override;
	IImageResource& getLastRenderedImage();
	IImageResource& getLastRenderedDepthBuffer() override;
private:
	void createSemaphores(const vk::UniqueDevice&);

	template<vk::ImageLayout from, vk::ImageLayout to>
	static void transitionImageResources(const CommandBuffer&, const vk::Queue&, std::set<ImageResource*> resources);

	SwapChain& m_swapChain;
	vk::Queue m_vkGraphicsQueue;
	vk::Queue m_vkPresentQueue;
	vk::UniqueSemaphore m_vkRenderFinishSemaphore;
	vk::UniqueSemaphore m_vkImageAvailableSemaphore;
	const Device& m_device;

	std::set<Resource*> m_submittedResources;
};
