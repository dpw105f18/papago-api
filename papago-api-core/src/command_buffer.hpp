#pragma once
#include "standard_header.hpp"
#include "sub_command_buffer.hpp"
#include "api_enums.hpp"

class ShaderProgram;

class CommandBuffer
{
public:

	//TODO: remove "override"s - place functionality in SubCommandBuffer or redesign relationship. -AM
	void begin(const RenderPass&);
	void begin(RenderPass&, SwapChain&, uint32_t imageIndex);
	void begin(const RenderPass&, ImageResource& depthStencilBuffer);
	void begin(const RenderPass&, SwapChain&, ImageResource& depthStencilBuffer);

	void end();
	void setPrimitiveTopology(Usage);
	void clearDepthBuffer(float value);
	void clearFrameBuffer(Color);
	void setDepthTest(DepthTest);
	void setUniform(const std::string&, const BufferResource&);
	void setUniform(const std::string&, const ImageResource&, Sampler&);
	void setInput(const BufferResource&);
	void setInterleavedInput(const std::vector<const std::string>&, const Resource&);
	void setIndexBuffer(const BufferResource&);
	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);
	void drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0);
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
	CommandBuffer(const vk::UniqueDevice& device, int queueFamilyIndex, Usage);
	
	// TODO: Have program so that we only need to pass in the name - Brandborg
	long getBinding(const ShaderProgram& program, const std::string& name);

	vk::UniqueCommandPool m_vkCommandPool;	//TODO: <-- make non-unique if we reuse command pools. -AM
	vk::UniqueCommandBuffer m_vkCommandBuffer;
	Usage m_usage; //TODO: <-- use this! -AM

	const vk::UniqueDevice& m_vkDevice;	//<-- used to update vkDescriptorSets. -AM
	//TODO: Check that this is not null, when calling non-begin methods on the object. - Brandborg
	RenderPass* m_renderPassPtr;
	

	friend class Device;
};