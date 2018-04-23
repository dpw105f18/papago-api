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

	void upload(const std::vector<char>& data) override; 

	std::vector<char> download() override;

	uint32_t getWidth() const;
	uint32_t getHeight() const;
	Format getFormat() const;

	void destroy() override;

private:
	static ImageResource createDepthResource(
		const Device& device,
		vk::Extent3D, 
		const std::vector<Format>& formatCandidates);

	static ImageResource createColorResource(
		vk::Image, 
		const Device& device,
		Format,
		vk::Extent3D);

	ImageResource(
		vk::Image&,
		const Device&,
		vk::ImageAspectFlags,
		Format,
		vk::Extent3D,
		vk::MemoryRequirements);

	ImageResource(
		vk::Image&, 
		const Device&, 
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

	vk::UniqueFramebuffer& createFramebuffer(vk::RenderPass& renderPass);

	vk::Image m_vkImage;
	vk::UniqueImageView m_vkImageView;
	Format m_format;
	vk::Extent3D m_vkExtent;
	const Device& m_device;
	vk::UniqueFramebuffer m_vkFramebuffer;

	friend class SwapChain;
	friend class Device;
	friend class CommandBuffer;
	friend class RenderPass;
};

//create
template<>
inline void ImageResource::transition<vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eUndefined, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1} //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal>(const CommandBuffer& commandBuffer)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eTransferDstOptimal,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

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
inline void ImageResource::transition<vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral>(const CommandBuffer& commandBuffer)
{
	auto imageMemoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(),
		vk::AccessFlags(),
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eGeneral,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vkImage,
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

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
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::eTransferSrcOptimal, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eTransferSrcOptimal, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::eShaderReadOnlyOptimal, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eShaderReadOnlyOptimal, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::eGeneral, //oldLayout
		vk::ImageLayout::ePresentSrcKHR, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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
inline void ImageResource::transition<vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral>(const CommandBuffer& commandBuffer)
{
	auto imageMemeoryBarrier = vk::ImageMemoryBarrier(
		vk::AccessFlags(), //srcAccessMask
		vk::AccessFlags(), //dstAccessMask
		vk::ImageLayout::ePresentSrcKHR, //oldLayout
		vk::ImageLayout::eGeneral, //newLayout
		VK_QUEUE_FAMILY_IGNORED,	//srcQueueFamilyIndex
		VK_QUEUE_FAMILY_IGNORED,	//dstQueueFamliyIndex
		m_vkImage, //image
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } //subresourceRange
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

//depth resource
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

//NOTE: this must always be after the other template definitions!
template<vk::ImageLayout source, vk::ImageLayout destination>
void ImageResource::transition(const CommandBuffer &)
{
	std::stringstream stream;

	stream << "Transition from " << to_string(source) << " to " << to_string(destination) << " not implemented.";
	PAPAGO_ERROR(stream.str());
}
