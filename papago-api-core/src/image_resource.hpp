#pragma once
#include "standard_header.hpp"
#include "resource.hpp"
#include "device.hpp"
#include "swap_chain.hpp"

class ImageResource : public Resource
{
public:

	// Inherited via Resource
	void upload(std::vector<char> data) override;
	void destroy() override;
	void download() override;


private:
	static ImageResource createDepthResource(const vk::PhysicalDevice &physicalDevice, const vk::Device &device, size_t width, size_t height, const std::vector<Format>& formatCandidates);
	static ImageResource createColorResource(vk::Image, const vk::Device&, Format format);
	static ImageResource createTextureResource();

	ImageResource(vk::ImageCreateInfo, const vk::PhysicalDevice&, const vk::Device&);
	ImageResource(vk::Image, const vk::Device&, Format format);

	static Format findSupportedFormat(const vk::PhysicalDevice&, const std::vector<Format>&, vk::ImageTiling, vk::FormatFeatureFlags);
	void setImageView(const vk::Device&);

	vk::ImageCreateInfo m_VkCreateInfo;
	vk::Image m_VkImage;
	vk::DeviceMemory m_VkMemory;
	vk::ImageView m_VkImageView;
	Format m_Format;

	//friend ImageResource Device::createDepthResource(size_t width, size_t height, TypeEnums);
	friend class SwapChain; //TODO: Figure out how to be friend of private constructor instead.
	friend class Device;
};