#pragma once
#include "standard_header.hpp"
#include "sub_command_buffer.hpp"
#include "api_enums.hpp"

class CommandBuffer
{
public:

	//TODO: remove "override"s - place functionality in SubCommandBuffer or redesign relationship. -AM
	void begin(const RenderPass&);
	void begin(RenderPass&, SwapChain&);
	void begin(const RenderPass&, ImageResource& depthStencilBuffer);
	void begin(const RenderPass&, SwapChain&, ImageResource& depthStencilBuffer);

	void end();
	void setPrimitiveTopology(Usage);
	void clearDepthBuffer(float value);
	void clearFrameBuffer(Color);
	void setDepthTest(DepthTest);
	void setUniform(const std::string&, const BufferResource&);
	void setUniform(const std::string&, const ImageResource&, const Sampler2D&);
	void setInput(const std::string&, const Resource&);
	void setInterleavedInput(const std::vector<const std::string>&, const Resource&);
	void setIndexBuffer(const Resource&);
	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);
	void setOutput(const std::string&, ImageResource&);
	void executeSubCommands(std::vector<SubCommandBuffer>);

	explicit operator vk::CommandBuffer&();

private:
	CommandBuffer(const vk::UniqueDevice& device, int queueFamilyIndex, Usage);

	vk::UniqueCommandBuffer m_vkCommandBuffer;
	vk::UniqueCommandPool m_vkCommandPool;	//TODO: <-- make non-unique if we reuse command pools. -AM
	Usage m_usage; //TODO: <-- use this! -AM

	friend class Device;
};