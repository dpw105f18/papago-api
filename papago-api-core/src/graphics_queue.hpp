#pragma once
#include <vector>
#include <set>
#include "resource.hpp"
#include "igraphics_queue.hpp"

class Device;
class CommandBuffer;
class ISwapChain;
class ICommandBuffer;
class ImageResource;

class GraphicsQueue : public IGraphicsQueue
{
public:
	GraphicsQueue(const Device&, int graphicsQueueIndex, int presentQueueIndex);
	
	void present(ISwapchain& swapchain) override;
	void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) override;
	void submitPresent(const std::vector<std::reference_wrapper<ICommandBuffer>>&, ISwapchain &) override;
private:
	void createSemaphores(const vk::UniqueDevice&);

	template<vk::ImageLayout from, vk::ImageLayout to>
	static void transitionImageResources(const CommandBuffer&, const vk::Queue&, std::set<ImageResource*> resources);

	vk::Queue m_vkGraphicsQueue;
	vk::Queue m_vkPresentQueue;
	vk::UniqueSemaphore m_vkRenderFinishSemaphore;
	const Device& m_device;
	std::vector<vk::UniqueFence> m_vkFences;

	std::set<Resource*> m_submittedResources;

};
