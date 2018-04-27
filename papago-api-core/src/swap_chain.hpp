#pragma once
#include "device.hpp"
#include "image_resource.hpp"
#include "iswapchain.hpp"

class SwapChain : public ISwapchain {
public:
	SwapChain(vk::UniqueDevice&, vk::UniqueSwapchainKHR&, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D);
	
	explicit operator vk::SwapchainKHR&();

	uint32_t getWidth() const override;
	uint32_t getHeight() const override;
	Format getFormat() const override;

private:
	vk::UniqueSwapchainKHR m_vkSwapChain;
	std::vector<ImageResource> m_colorResources;
	std::vector<ImageResource> m_depthResources;
	std::vector<vk::UniqueFramebuffer> m_vkFramebuffers;
	vk::UniqueRenderPass m_vkRenderPass;
	vk::Extent2D m_vkExtent;
	std::vector<vk::UniqueFence> m_vkFences;

	
	vk::UniqueRenderPass createDummyRenderPass(const vk::UniqueDevice& device); //<-- TODO: use proper RenderPass


	//TODO: use ImageResource when it's done?
	friend class Device;

	friend class CommandBuffer; //<-- used to get vk::Framebuffer's

	friend class GraphicsQueue;
};
