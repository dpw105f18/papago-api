#pragma once
#include "standard_header.hpp"
#include "resource.hpp"
#include "device.hpp"

class ImageResource : public Resource
{
public:

	// Inherited via Resource
	void upload(const std::vector<char>& data) override;
	void destroy() override;
	std::vector<char> download() override;


private:
	static ImageResource createDepthResource(const vk::PhysicalDevice &physicalDevice, const vk::UniqueDevice &device, size_t width, size_t height, const std::vector<Format>& formatCandidates);
	static ImageResource createColorResource(vk::Image, const vk::UniqueDevice&, Format format);
	static ImageResource createTextureResource();

	ImageResource(vk::ImageCreateInfo, const vk::PhysicalDevice&, const vk::UniqueDevice&, vk::ImageAspectFlags);
	ImageResource(vk::Image, const vk::UniqueDevice&, Format format);

	static Format findSupportedFormat(const vk::PhysicalDevice&, const std::vector<Format>&, vk::ImageTiling, vk::FormatFeatureFlags);
	void setImageView(const vk::UniqueDevice&);

	vk::ImageCreateInfo m_vkCreateInfo;
	vk::Image m_vkImage;

	vk::DeviceMemory m_vkMemory;
	vk::ImageView m_vkImageView;
	Format m_format;

	friend class SwapChain; //TODO: Figure out how to be friend of private constructor instead.
	friend class Device;
};
