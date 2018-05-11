#pragma once
#include "icommand_buffer.hpp"
#include "recording_command_buffer.hpp"
#include <string>
#include <vector>

class IImageResource;
class ImageResource;
class BufferResource;
class Resource;
class Sampler;
class SwapChain;
class Device;
class CommandBuffer;

class SubCommandBuffer : public ISubCommandBuffer, public CommandRecorder<IRecordingSubCommandBuffer>
{
public:
	SubCommandBuffer(const vk::UniqueDevice&, uint32_t queueFamilyIndex);

	explicit operator vk::CommandBuffer&();
	
	// Inherited via ISubCommandBuffer
	void record(IRenderPass &, std::function<void(IRecordingSubCommandBuffer&)>) override;

	IRecordingSubCommandBuffer& setVertexBuffer(IBufferResource &) override;
	IRecordingSubCommandBuffer& setIndexBuffer(IBufferResource &) override;

	/*
	//defined in IRecordingSubCommandBuffer, implemented in CommandRecorder<IRecordingSubCommandBuffer>:
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::setInput(IBufferResource&) override;
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::setUniform(const std::string& uniformName, IImageResource&, ISampler&) override;
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::setUniform(const std::string& uniformName, IBufferResource&) override;
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::setUniform(const std::string& uniformName, DynamicBuffer&, size_t) override;
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::setIndexBuffer(IBufferResource&) override;
	IRecordingSubCommandBuffer& IRecorder<IRecordingSubCommandBuffer>::drawIndexed(size_t indexCount, size_t instanceCount = 1, size_t firstIndex = 0, size_t vertexOffset = 0, size_t firstInstance = 0) override;
	*/

private:

	void begin();
	void end();
};