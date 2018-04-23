#pragma once
#include "standard_header.hpp"
#include "device.hpp"
#include "image_resource.hpp"

class SwapChain {
public:
	explicit operator vk::SwapchainKHR&();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	Format getFormat() const;

private:
	vk::UniqueSwapchainKHR m_vkSwapChain;
	std::vector<ImageResource> m_colorResources;
	std::vector<ImageResource> m_depthResources;
	std::vector<vk::UniqueFramebuffer> m_vkFramebuffers;
	vk::UniqueRenderPass m_vkRenderPass;
	vk::Extent2D m_vkExtent;
	std::vector<vk::UniqueFence> m_vkFences;

	SwapChain(vk::UniqueDevice&, vk::UniqueSwapchainKHR&, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D);
	
	vk::UniqueRenderPass createDummyRenderPass(const vk::UniqueDevice& device); //<-- TODO: use proper RenderPass


	//TODO: use ImageResource when it's done?
	friend class Device;

	friend class CommandBuffer; //<-- used to get vk::Framebuffer's

	friend class GraphicsQueue;
};
