#pragma once
#include <functional>
#include "ibuffer_resource.hpp"
#include "iimage_resource.hpp"

class IRecordingCommandBuffer;
class ISampler;
class IRenderPass;
class ISwapchain;

class ICommandBuffer {
public:
	virtual ~ICommandBuffer() = default;
	virtual void record(IRenderPass&, ISwapchain&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource& color, IImageResource& depth, std::function<void(IRecordingCommandBuffer&)>) = 0;
};

class IRecordingCommandBuffer {
public:
	virtual ~IRecordingCommandBuffer() = default;

	virtual IRecordingCommandBuffer& setInput(IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& setUniform(const std::string& uniformName, IImageResource&, ISampler&) = 0;
	virtual IRecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& setIndexBuffer(IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
	
	virtual IRecordingCommandBuffer& clearColorBuffer(float red, float green, float blue, float alpha) = 0;
	virtual IRecordingCommandBuffer& clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha) = 0;
	virtual IRecordingCommandBuffer& clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) = 0;
	virtual IRecordingCommandBuffer& clearDepthStencilBuffer(float depth, uint32_t stencil) = 0;
	virtual IRecordingCommandBuffer& clearDepthBuffer(float value) = 0;
	virtual IRecordingCommandBuffer& clearStencilBuffer(uint32_t value) = 0;
};
