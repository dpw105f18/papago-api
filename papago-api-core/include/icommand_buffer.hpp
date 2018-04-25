#pragma once
#include <functional>

class RecordingCommandBuffer;
class IImageResource;
class ISampler;
class IRenderPass;
class IFramebuffer;

class ICommandBuffer {
public:
	~ICommandBuffer() = default;
	//virtual void record(IRenderPass&, IFramebuffer&, std::function<void(RecordingCommandBuffer&)>) = 0;
};

class RecordingCommandBuffer {
public:
	~RecordingCommandBuffer() = default;

	virtual RecordingCommandBuffer& setInput() = 0;
	virtual RecordingCommandBuffer& setUniform(const std::string& uniformName, IImageResource&, ISampler&) = 0;
	virtual RecordingCommandBuffer& setUniform(const std::string& uniformName, IBufferResource&) = 0;
	virtual RecordingCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
};