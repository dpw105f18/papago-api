#pragma once

#include "resource.hpp"
#include "iimage_resource.hpp"

class CommandBuffer;
class Device;

class ImageResource : public Resource, public IImageResource
{
public:
	ImageResource(const ImageResource&) = delete;
	ImageResource(ImageResource&& other) noexcept;
	~ImageResource();

	template<vk::ImageLayout source, vk::ImageLayout destination>
	void transition(const CommandBuffer&);

	void upload(const std::vector<char>& data) override; 

	std::vector<char> download();

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	Format getFormat() const override;

	void destroy() override;

	ImageResource(
		vk::Image&,
		const Device&,
		vk::ImageAspectFlags,
		vk::Format,
		vk::Extent3D,
		vk::MemoryRequirements);

	ImageResource(
		vk::Image&, 
		const Device&, 
		vk::Format,
		vk::Extent3D);

	bool inUse() override;
	vk::UniqueFramebuffer& createFramebuffer(vk::RenderPass& renderPass);

	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	vk::Format m_format;
	vk::Extent3D m_vkExtent;
	vk::UniqueFramebuffer m_vkFramebuffer;
	static ImageResource createDepthResource(
		const Device& device,
		vk::Extent3D, 
		const std::vector<vk::Format>& formatCandidates);

	static ImageResource createColorResource(
		vk::Image, 
		const Device& device,
		vk::Format,
		vk::Extent3D);

	static vk::Format findSupportedFormat(
		const vk::PhysicalDevice&, 
		const std::vector<vk::Format>&, 
		vk::ImageTiling, 
		vk::FormatFeatureFlags);

private:
	void createImageView(
		const vk::UniqueDevice&, 
		vk::ImageAspectFlags = vk::ImageAspectFlagBits::eColor);

	const Device& m_device;
};