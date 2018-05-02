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
	void transition(const CommandBuffer&, vk::AccessFlags srcAccessFlags = vk::AccessFlags(), vk::AccessFlags dstAccessFlags = vk::AccessFlags());

	void upload(const std::vector<char>& data) override; 

	std::vector<char> download() override;

	uint32_t getWidth() const override;
	uint32_t getHeight() const override;
	Format getFormat() const override;

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
	vk::UniqueFramebuffer m_vkFramebuffer;
	vk::ImageAspectFlags m_vkAspectFlags;

	friend class SwapChain;
	friend class Device;
	friend class CommandBuffer;
	friend class RenderPass;
};

//create
template<>
inline void ImageResource::transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eUndefined, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{m_vkAspectFlags, 0, 1, 0, 1} //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//pre-upload
template<>
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ m_vkAspectFlags, 0, 1, 0, 1 });

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eBottomOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlags(),
		{},
		{},
		{ imageMemoryBarrier });
}


//post-upload
template<>
inline void ImageResource::transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eGeneral,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ m_vkAspectFlags, 0, 1, 0, 1 });

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eBottomOfPipe,
		vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlags(),
		{},
		{},
		{ imageMemoryBarrier }
	);
}

//pre-download
template<>
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::eTransferSrcOptimal, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//post-download
template<>
inline void ImageResource::transition<vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eTransferSrcOptimal, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//pre-setUniform
template<>
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::eShaderReadOnlyOptimal, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//post-setUniform
template<>
inline void ImageResource::transition<vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eShaderReadOnlyOptimal, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//pre-present
template<>
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::ePresentSrcKHR, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

//post-present
template<>
inline void ImageResource::transition<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		srcAccessFlags, //srcAccessMask
		dstAccessFlags, //dstAccessMask
		vk::ImageLayout::ePresentSrcKHR, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ m_vkAspectFlags, 0, 1, 0, 1 } //subresourceRange
	);

	commandBuffer->pipelineBarrier(
		vk::PipelineStageFlagBits::eTopOfPipe, //srcStageMask
		vk::PipelineStageFlagBits::eBottomOfPipe, //dstStageMask
		vk::DependencyFlags(), //dependencyFlags
		{}, //memoryBarriers
		{}, //bufferMemoryBarriers
		{ imageMemeoryBarrier } //imageMemoryBarriers
	);
}

/*
//depth resource
template<>
inline void ImageResource::transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal>(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
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
*/

//NOTE: this must always be after the other template definitions!
template<vk::ImageLayout source, vk::ImageLayout destination>
void ImageResource::transition(
	const CommandBuffer& commandBuffer
	, vk::AccessFlags srcAccessFlags
	, vk::AccessFlags dstAccessFlags)
{
	std::stringstream stream;

	stream << "Transition from " << to_string(source) << " to " << to_string(destination) << " not implemented.";
	PAPAGO_ERROR(stream.str());
}
