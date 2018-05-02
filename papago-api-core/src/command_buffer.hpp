#pragma once
#include <set>
#include "sub_command_buffer.hpp"
#include "api_enums.hpp"
#include "icommand_buffer.hpp"

class ShaderProgram;

class CommandBuffer : public ICommandBuffer, public IRecordingCommandBuffer
{
public:
	CommandBuffer(const vk::UniqueDevice& device, int queueFamilyIndex, Usage);

	void begin(RenderPass&, const vk::UniqueFramebuffer&, vk::Extent2D);
	void end();
	void setPrimitiveTopology(Usage);
	void setDepthTest(DepthTest);

	//Inherited from ICommandBuffer
	void record(IRenderPass&, ISwapchain&, size_t frameIndex, std::function<void(IRecordingCommandBuffer&)>) override;
	void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) override;
	void record(IRenderPass &, IImageResource & color, IImageResource & depth, std::function<void(IRecordingCommandBuffer&)>) override;

	//Inherited from IRecordingCommandBuffer
	IRecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) override;
	IRecordingCommandBuffer& setUniform(const std::string&, IImageResource&, ISampler&) override;
	IRecordingCommandBuffer& setInput(IBufferResource&) override;
	IRecordingCommandBuffer& setIndexBuffer(IBufferResource&) override;
	IRecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance) override;
	IRecordingCommandBuffer& clearColorBuffer(float red, float green, float blue, float alpha) override;
	IRecordingCommandBuffer& clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha) override;
	IRecordingCommandBuffer& clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) override;
	IRecordingCommandBuffer& clearDepthStencilBuffer(float depth, uint32_t stencil) override;
	IRecordingCommandBuffer& clearDepthBuffer(float value) override;
	IRecordingCommandBuffer& clearStencilBuffer(uint32_t value) override;

	void setInterleavedInput(const std::vector<const std::string>&, const Resource&);
	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);
	void setOutput(const std::string&, ImageResource&);
	void executeSubCommands(std::vector<SubCommandBuffer>);

	explicit operator vk::CommandBuffer&();
	const vk::CommandBuffer* operator ->() const {
		return &*m_vkCommandBuffer;
	}
	const vk::CommandBuffer& operator *() const {
		return *m_vkCommandBuffer;
	}

private:

	long getBinding(const std::string& name);

	vk::UniqueCommandPool m_vkCommandPool;	//TODO: <-- make non-unique if we reuse command pools. -AM
	vk::UniqueCommandBuffer m_vkCommandBuffer;
	Usage m_usage; //TODO: <-- use this! -AM

	const vk::UniqueDevice& m_vkDevice;	//<-- used to update vkDescriptorSets. -AM
	
	// TODO: Another approach could be to create another interface and expose it via builder pattern or lambda expressions - CW 2018-04-23
	RenderPass* m_renderPassPtr;
	vk::Extent2D m_vkCurrentRenderTargetExtent;
	std::set<Resource*> m_resourcesInUse;
	
	friend class Device;
	friend class GraphicsQueue;

	void clearAttatchment(const vk::ClearValue&, vk::ImageAspectFlags);

};