#pragma once
#include <functional>
#include <vector>

class IRecordingCommandBuffer;
class IRecordingSubCommandBuffer;
class ISampler;
class IRenderPass;
class ISwapchain;
class ISubCommandBuffer;
class IImageResource;
class IBufferResource;
class DynamicBufferResource;
class IParameterBlock;

class ICommandBuffer {
public:
	virtual ~ICommandBuffer() = default;
	virtual void record(IRenderPass&, ISwapchain&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource&, std::function<void(IRecordingCommandBuffer&)>) = 0;
	virtual void record(IRenderPass&, IImageResource& color, IImageResource& depth, std::function<void(IRecordingCommandBuffer&)>) = 0;
};
class ISubCommandBuffer
{
public:
	virtual ~ISubCommandBuffer() = default;

	virtual void record(IRenderPass&, std::function<void(IRecordingSubCommandBuffer&)>) = 0;
};

template<class T>
class IRecorder
{
public:
	virtual ~IRecorder() = default;

	virtual T& setDynamicIndex(IParameterBlock& parameterBlock, const std::string& uniformName, size_t) = 0;
};

class IRecordingCommandBuffer 
	: public IRecorder<IRecordingCommandBuffer>
{
public:
	virtual ~IRecordingCommandBuffer() = default;

	virtual IRecordingCommandBuffer& execute(const std::vector<std::reference_wrapper<ISubCommandBuffer>>&) = 0;

	virtual IRecordingCommandBuffer& clearColorBuffer(float red, float green, float blue, float alpha) = 0;
	virtual IRecordingCommandBuffer& clearColorBuffer(int32_t red, int32_t green, int32_t blue, int32_t alpha) = 0;
	virtual IRecordingCommandBuffer& clearColorBuffer(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha) = 0;
	virtual IRecordingCommandBuffer& clearDepthStencilBuffer(float depth, uint32_t stencil) = 0;
	virtual IRecordingCommandBuffer& clearDepthBuffer(float value) = 0;
	virtual IRecordingCommandBuffer& clearStencilBuffer(uint32_t value) = 0;
};

class IRecordingSubCommandBuffer 
	: public IRecorder<IRecordingSubCommandBuffer> 
{
public:
	virtual ~IRecordingSubCommandBuffer() = default;

	virtual IRecordingSubCommandBuffer& drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) = 0;
	virtual IRecordingSubCommandBuffer& draw(size_t vertexCount, size_t instanceCount = 1, size_t firstVertex = 0, size_t firstInstance = 0) = 0;
	virtual IRecordingSubCommandBuffer& setVertexBuffer(IBufferResource&) = 0;
	virtual IRecordingSubCommandBuffer& setIndexBuffer(IBufferResource&) = 0;
	virtual IRecordingSubCommandBuffer& setParameterBlock(IParameterBlock&) = 0;
};
