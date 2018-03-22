#pragma once
#include "standard_header.hpp"
#include "device.hpp"

class SwapChain {
public:
	void present();
private:
	vk::UniqueSwapchainKHR m_VkSwapChain;
	vk::Extent2D m_VkExtent;
	Format& m_Format;
	std::vector<vk::Image> m_Images;
	std::vector<vk::UniqueImageView> m_ImageViews;
	std::vector<vk::UniqueFramebuffer> m_Framebuffers;
	
	SwapChain(vk::UniqueDevice&, vk::UniqueSwapchainKHR&, Format, vk::Extent2D);
	

	//TODO: finish diz (use ImageResource when it's done?):
	vk::UniqueImage createDepthImage(const vk::PhysicalDevice&, const vk::UniqueDevice&) const;

	static Format findSupportedFormat(const vk::PhysicalDevice&, const std::vector<Format>& candidateFormats, vk::ImageTiling, vk::FormatFeatureFlags);
	
	friend SwapChain Device::createSwapChain(const Format& , size_t ,SwapChainPresentMode, Surface&);
};