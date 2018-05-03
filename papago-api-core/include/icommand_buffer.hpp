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
	virtual void record(IRenderPass&, ISwapchain&, size_t frameIndex, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) = 0;
};

class IRecordingCommandBuffer {
public:
	virtual ~IRecordingCommandBuffer() = default;

	virtual IRecordingCommandBuffer& setInput(IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& setUniform(const std::string& uniformName, IImageResource&, ISampler&) = 0;
	virtual IRecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& setUniform(const std::string& uniformName, DynamicBuffer&) = 0;
	virtual IRecordingCommandBuffer& setIndexBuffer(IBufferResource&) = 0;
	virtual IRecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
	
};
