#pragma once
#include "standard_header.hpp"
#include "device.hpp"

class SwapChain {
public:
	void present();
private:
	vk::SwapchainKHR& m_VkSwapChain;
	Format& m_Format;
	std::vector<vk::Image> m_Images;
	std::vector<vk::ImageView> m_ImageViews;
	
	SwapChain(vk::Device&, vk::SwapchainKHR&, Format&);
	friend SwapChain Device::createSwapChain(Format , size_t ,SwapChainPresentMode, Surface&);
	
};