#pragma once
#include "standard_header.hpp"
#include "resource.hpp"
#include "device.hpp"

class ImageResource : public Resource
{
public:
	ImageResource(const ImageResource&) = delete;
	ImageResource(ImageResource&& other) noexcept;
	~ImageResource();

	// Inherited via Resource
	void destroy() override;

private:
	static ImageResource createDepthResource(
		const vk::PhysicalDevice&, 
		const vk::UniqueDevice&, 
		size_t width, size_t height, 
		const std::vector<Format>& formatCandidates);

	static ImageResource createColorResource(
		vk::Image, 
		const vk::UniqueDevice&,
		Format);

	ImageResource(
		vk::Image&,
		const vk::PhysicalDevice&,
		const vk::UniqueDevice&,
		vk::ImageAspectFlags,
		Format,
		vk::MemoryRequirements);

	ImageResource(
		vk::Image&, 
		const vk::UniqueDevice&, 
		Format);


	static Format findSupportedFormat(
		const vk::PhysicalDevice&, 
		const std::vector<Format>&, 
		vk::ImageTiling, 
		vk::FormatFeatureFlags);

	void createImageView(
		const vk::UniqueDevice&, 
		vk::ImageAspectFlags = vk::ImageAspectFlagBits::eColor);

	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	Format m_format;

	friend class SwapChain;
	friend class Device;
};
