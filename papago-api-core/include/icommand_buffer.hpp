#pragma once
#include <functional>

class IRecordingCommandBuffer;
class IRecordingSubCommandBuffer;
class ISampler;
class IRenderPass;
class ISwapchain;
class ISubCommandBuffer;
class IImageResource;
class IBufferResource;
class DynamicBuffer;

class ICommandBuffer {
public:
	virtual ~ICommandBuffer() = default;
	virtual void record(IRenderPass&, ISwapchain&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource& color, IImageResource& depth, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual std::unique_ptr<ISubCommandBuffer> createSubCommandBuffer() = 0; //TODO: handle sub-cmd usage
};

class ISubCommandBuffer
{
public:
	virtual ~ISubCommandBuffer() = default;

	virtual void record(IRenderPass&, ISwapchain&, size_t frameIndex, std::function<void(IRecordingSubCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(IRecordingSubCommandBuffer&)>) = 0;
};

template<class T>
class IRecorder
{
public:
	virtual ~IRecorder() = default;

	virtual T& setInput(IBufferResource&) = 0;
	virtual T& setUniform(const std::string& uniformName, IImageResource&, ISampler&) = 0;
	virtual T& setUniform(const std::string& uniformName, IBufferResource&) = 0;
	virtual T& setUniform(const std::string& uniformName, DynamicBuffer&, size_t) = 0;
	virtual T& setIndexBuffer(IBufferResource&) = 0;
	virtual T& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
};

class IRecordingCommandBuffer 
	: public IRecorder<IRecordingCommandBuffer>
{
public:
	virtual ~IRecordingCommandBuffer() = default;

	virtual IRecordingCommandBuffer& execute(std::vector<std::unique_ptr<ISubCommandBuffer>>&) = 0;
};

class IRecordingSubCommandBuffer 
	: public IRecorder<IRecordingSubCommandBuffer> 
{
public:
	virtual ~IRecordingSubCommandBuffer() = default;
};
