#pragma once
#include <functional>
#include "ibuffer_resource.hpp"
#include "iimage_resource.hpp"

class RecordingCommandBuffer;
class ISampler;
class IRenderPass;
class ISwapchain;

class ICommandBuffer {
public:
	virtual ~ICommandBuffer() = default;
	virtual void record(IRenderPass&, ISwapchain&, size_t frameIndex, std::function<void(RecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(RecordingCommandBuffer&)>) = 0;
};

class RecordingCommandBuffer {
public:
	virtual ~RecordingCommandBuffer() = default;

	virtual RecordingCommandBuffer& setInput(IBufferResource&) = 0;
	virtual RecordingCommandBuffer& setUniform(const std::string& uniformName, IImageResource&, ISampler&) = 0;
	virtual RecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) = 0;
	virtual RecordingCommandBuffer& setIndexBuffer(IBufferResource&) = 0;
	virtual RecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
	
};
