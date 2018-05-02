#pragma once
class ICommandBuffer;
class IImageResource;

class IGraphicsQueue
{
public:
	virtual ~IGraphicsQueue() = default;

	virtual size_t getNextFrameIndex() = 0;
	virtual void present() = 0;
	virtual void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) = 0;
	virtual IImageResource& getLastRenderedDepthBuffer() = 0;
	virtual IImageResource& getLastRenderedImage() = 0;
};
