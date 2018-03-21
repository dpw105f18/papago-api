#pragma once
#include "standard_header.hpp"
#include "device.hpp"

class SwapChain {
public:
	void present();
private:
	vk::SwapchainKHR& m_VkSwapChain;
	std::vector<ImageResource> m_colorResources;
	std::vector<ImageResource> m_depthResources;
	std::vector<vk::Framebuffer> m_Framebuffers;
	
	SwapChain(vk::Device&, vk::SwapchainKHR&, std::vector<ImageResource>& colorResources, std::vector<ImageResource>& depthResources, vk::Extent2D);
	
	vk::RenderPass createDummyRenderPass(const vk::Device& device); //<-- TODO: use proper RenderPass

	//TODO: finish diz (use ImageResource when it's done?):
	friend SwapChain Device::createSwapChain(const Format& , size_t ,SwapChainPresentMode, Surface&);
};