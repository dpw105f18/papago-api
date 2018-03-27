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
		vk::Extent3D, 
		const std::vector<Format>& formatCandidates);

	static ImageResource createColorResource(
		vk::Image, 
		const vk::UniqueDevice&,
		Format,
		vk::Extent3D);

	ImageResource(
		vk::Image&,
		const vk::PhysicalDevice&,
		const vk::UniqueDevice&,
		vk::ImageAspectFlags,
		Format,
		vk::Extent3D,
		vk::MemoryRequirements);

	ImageResource(
		vk::Image&, 
		const vk::UniqueDevice&, 
		Format,
		vk::Extent3D);


	static Format findSupportedFormat(
		const vk::PhysicalDevice&, 
		const std::vector<Format>&, 
		vk::ImageTiling, 
		vk::FormatFeatureFlags);

	void createImageView(
		const vk::UniqueDevice&, 
		vk::ImageAspectFlags = vk::ImageAspectFlagBits::eColor);

	vk::ImageCreateInfo m_vkImageCreateInfo;
	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	Format m_format;
	vk::Extent3D m_vkExtent;

	friend class SwapChain;
	friend class Device;
};
