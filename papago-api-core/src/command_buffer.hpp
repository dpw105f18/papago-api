#pragma once
#include "sub_command_buffer.hpp"

class CommandBuffer : public SubCommandBuffer
{
public:

	void begin(const RenderPass&) override;
	void begin(const RenderPass&, SwapChain&) override;
	void begin(const RenderPass&, ImageResource& depthStencilBuffer) override;
	void begin(const RenderPass&, SwapChain&, ImageResource& depthStencilBuffer) override;

	void end() override;
	void setPrimitiveTopology(Usage) override;
	void clearDepthBuffer(float value) override;
	void clearFrameBuffer(Color) override;
	void setDepthTest(DepthTest) override;
	void setUniform(const std::string&, const BufferResource&) override;
	void setUniform(const std::string&, const ImageResource&, const Sampler2D&) override;
	void setInput(const std::string&, const Resource&) override;
	void setInterleavedInput(const std::vector<const std::string>&, const Resource&) override;
	void setIndexBuffer(const Resource&) override;
	void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation) override;
	void setOutput(const std::string&, ImageResource&) override;
	void executeSubCommands(std::vector<SubCommandBuffer>);
};