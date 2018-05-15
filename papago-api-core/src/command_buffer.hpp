#pragma once
#include <mutex>
#include "recording_command_buffer.hpp"
#include "icommand_buffer.hpp"

class SwapChain;
class ImageResource;

class CommandBuffer : public CommandRecorder<IRecordingCommandBuffer>, public ICommandBuffer
{
public:
	CommandBuffer(const vk::UniqueDevice& device, int queueFamilyIndex);
	CommandBuffer(CommandBuffer&&);

	void record(IRenderPass&, ISwapchain&, std::function<void(IRecordingCommandBuffer&)>) override;
	void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) override;
	void record(IRenderPass &, IImageResource & color, IImageResource & depth, std::function<void(IRecordingCommandBuffer&)>) override;

	IRecordingCommandBuffer& execute(std::vector<std::unique_ptr<ISubCommandBuffer>>&) override;

	void begin(RenderPass&, const vk::UniqueFramebuffer&, vk::Extent2D);	//TODO: <-- remove imageIndex. -AM
	void end();

	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);

	IRecordingCommandBuffer& clearColorBuffer(float red, float green, float blue, float alpha) override;
	IRecordingCommandBuffer& clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha) override;
	IRecordingCommandBuffer& clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) override;
	IRecordingCommandBuffer& clearDepthStencilBuffer(float depth, uint32_t stencil) override;
	IRecordingCommandBuffer& clearDepthBuffer(float value) override;
	IRecordingCommandBuffer& clearStencilBuffer(uint32_t value) override;
	
	explicit operator vk::CommandBuffer&();
	const vk::CommandBuffer* operator ->() const {
		return &*m_vkCommandBuffer;
	}
	const vk::CommandBuffer& operator *() const {
		return *m_vkCommandBuffer;
	}

	std::vector<uint32_t> m_boundDescriptorBindings; 
private:
	uint32_t m_queueFamilyIndex;
	void clearAttachment(const vk::ClearValue &, vk::ImageAspectFlags);
};