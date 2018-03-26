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
	std::vector<vk::Framebuffer> m_framebuffers;
	
	SwapChain(vk::UniqueDevice&, vk::UniqueSwapchainKHR&, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D);
	
	vk::RenderPass createDummyRenderPass(const vk::UniqueDevice& device); //<-- TODO: use proper RenderPass

	//TODO: use ImageResource when it's done?
	friend class Device;
};
