#pragma once
#include "standard_header.hpp"
#include "device.hpp"
#include "image_resource.hpp"

class SwapChain {
public:
	void present();
private:
	vk::UniqueSwapchainKHR m_vkSwapChain;
	std::vector<ImageResource> m_colorResources;
	std::vector<ImageResource> m_depthResources;
	std::vector<vk::UniqueFramebuffer> m_framebuffers;
	vk::UniqueRenderPass m_vkRenderPass;
	
	SwapChain(vk::UniqueDevice&, vk::UniqueSwapchainKHR&, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D);
	
	vk::UniqueRenderPass createDummyRenderPass(const vk::UniqueDevice& device); //<-- TODO: use proper RenderPass

	//TODO: use ImageResource when it's done?
	friend class Device;
};
