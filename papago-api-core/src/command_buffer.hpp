#pragma once
#include <set>
#include "sub_command_buffer.hpp"
#include "api_enums.hpp"
#include "icommand_buffer.hpp"

class ShaderProgram;

class CommandBuffer : public ICommandBuffer, public RecordingCommandBuffer
{
public:
	CommandBuffer(const vk::UniqueDevice& device, int queueFamilyIndex, Usage);

	void record(IRenderPass&, ISwapchain&, size_t frameIndex, std::function<void(RecordingCommandBuffer&)>) override;
	void record(IRenderPass&, IImageResource&, std::function<void(RecordingCommandBuffer&)>) override;

	//TODO: remove "override"s - place functionality in SubCommandBuffer or redesign relationship. -AM
	void begin(const RenderPass&);
	void begin(RenderPass&, SwapChain&, uint32_t imageIndex);	//TODO: <-- remove imageIndex. -AM
	void begin(RenderPass&, ImageResource& renderTarget);		//TODO: use Format and Extent iso. ImageResource? -AM
	void begin(const RenderPass&, SwapChain&, ImageResource& depthStencilBuffer);

	void end();
	void setPrimitiveTopology(Usage);
	void clearDepthBuffer(float value);
	void clearFrameBuffer(Color);
	void setDepthTest(DepthTest);
	RecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) override;
	RecordingCommandBuffer& setUniform(const std::string&, IImageResource&, ISampler&) override;
	RecordingCommandBuffer& setInput(IBufferResource&) override;
	RecordingCommandBuffer& setIndexBuffer(IBufferResource&) override;
	void setInterleavedInput(const std::vector<const std::string>&, const Resource&);
	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);
	RecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount, size_t firstIndex, size_t vertexOffset, size_t firstInstance) override;
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
	
	// TODO: Have program so that we only need to pass in the name - Brandborg
	long getBinding(const ShaderProgram& program, const std::string& name);

	vk::UniqueCommandPool m_vkCommandPool;	//TODO: <-- make non-unique if we reuse command pools. -AM
	vk::UniqueCommandBuffer m_vkCommandBuffer;
	Usage m_usage; //TODO: <-- use this! -AM

	const vk::UniqueDevice& m_vkDevice;	//<-- used to update vkDescriptorSets. -AM
	//TODO: Check that this is not null, when calling non-begin methods on the object. - Brandborg
	// TODO: Another approach could be to create another interface and expose it via builder pattern or lambda expressions - CW 2018-04-23
	RenderPass* m_renderPassPtr;

	std::set<Resource*> m_resourcesInUse;

	friend class Device;
	friend class GraphicsQueue;
};