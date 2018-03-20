#pragma once
#include "standard_header.hpp"
#include "device.hpp"

class SwapChain {
public:
	void present();
private:
	vk::SwapchainKHR& m_VkSwapChain;
	vk::Extent2D m_VkExtent;
	Format& m_Format;
	std::vector<vk::Image> m_Images;
	std::vector<vk::ImageView> m_ImageViews;
	std::vector<vk::Framebuffer> m_Framebuffers;
	
	SwapChain(vk::Device&, vk::SwapchainKHR&, Format, vk::Extent2D);
	

	//TODO: finish diz (use ImageResource when it's done?):
	vk::Image createDepthImage(const vk::PhysicalDevice&, const vk::Device&) const;


	Format findSupportedFormat(const vk::PhysicalDevice&, const std::vector<Format>& candidateFormats, vk::ImageTiling, vk::FormatFeatureFlags) const;
	
	friend SwapChain Device::createSwapChain(const Format& , size_t ,SwapChainPresentMode, Surface&);
};