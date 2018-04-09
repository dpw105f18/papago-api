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

	template<vk::ImageLayout source, vk::ImageLayout destination>
	void transition(const CommandBuffer&);

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

	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	Format m_format;
	vk::Extent3D m_vkExtent;

	friend class SwapChain;
	friend class Device;
	friend class CommandBuffer;
};

template<>
inline void ImageResource::transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal>(const CommandBuffer & commandBuffer)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlagBits::eTransferWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, 
		vk::PipelineStageFlagBits::eTransfer, 
		vk::DependencyFlags(), 
		{}, 
		{}, 
		{ imageMemoryBarrier });
}

template<>
inline void ImageResource::transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal>(const CommandBuffer & commandBuffer)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 });

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, 
		vk::PipelineStageFlagBits::eEarlyFragmentTests, 
		vk::DependencyFlags(), 
		{}, 
		{}, 
		{ imageMemoryBarrier });
}

template<>
inline void ImageResource::transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal>(const CommandBuffer & commandBuffer)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlagBits::eTransferWrite,
		vk::AccessFlagBits::eShaderRead,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, 
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags(), 
		{},
		{}, 
		{ imageMemoryBarrier });

}

template<vk::ImageLayout source, vk::ImageLayout destination>
void ImageResource::transition(const CommandBuffer &)
{
	std::stringstream stream;

	stream << "Transition from " << to_string(source) << " to " << to_string(destination) << " not implemented.";
	throw std::runtime_error(stream.str());
}
