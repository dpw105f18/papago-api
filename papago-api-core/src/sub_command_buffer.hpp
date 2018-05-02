#pragma once
#include "api_enums.hpp"
#include <string>
#include <vector>

class RenderPass;
class ImageResource;
class BufferResource;
class Resource;
class Sampler;
class SwapChain;

class SubCommandBuffer
{
public:

	virtual void begin(const RenderPass&);
	virtual void begin(const RenderPass&, SwapChain&);
	virtual void begin(const RenderPass&, ImageResource& depthStencilBuffer);
	virtual void begin(const RenderPass&, SwapChain&, ImageResource& depthStencilBuffer);

	virtual void end();
	virtual void setPrimitiveTopology(Usage);
	virtual void clearDepthBuffer(float value);
	virtual void setDepthTest(DepthTest);
	virtual void setUniform(const std::string&, const BufferResource&);
	virtual void setUniform(const std::string&, const ImageResource&, Sampler&);
	virtual void setInput(const std::string&, const Resource&);
	virtual void setInterleavedInput(const std::vector<const std::string>&, const Resource&);
	virtual void setIndexBuffer(const Resource&);
	virtual void drawInstanced(size_t instanceVertexCount, size_t instanceCount, size_t startVertexLocation, size_t startInstanceLocation);
	virtual void setOutput(const std::string&, ImageResource&);

	
};