#pragma once
#include "resource.hpp"
#include "iimage_resource.hpp"
#include "device.hpp"

class ImageResource : public Resource, public IImageResource
{
public:
	ImageResource(const ImageResource&) = delete;
	ImageResource(ImageResource&& other) noexcept;
	~ImageResource();

	template<vk::ImageLayout source, vk::ImageLayout destination>
	void transition(const CommandBuffer&);

	void upload(const std::vector<char>& data) override; 

	// Inherited via Resource
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
private:
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

	void createImageView(
		const vk::UniqueDevice&, 
		vk::ImageAspectFlags = vk::ImageAspectFlagBits::eColor);

	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	vk::Format m_format;
	vk::Extent3D m_vkExtent;
	const Device& m_device;

	friend class SwapChain;
	friend class Device;
	friend class CommandBuffer;
	friend class RenderPass;
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
