#pragma once
class ICommandBuffer;

class IGraphicsQueue
{
public:
	virtual ~IGraphicsQueue() = default;

	virtual size_t getNextFrameIndex() = 0;
	virtual void wait() = 0;
	virtual void present() = 0;
	virtual void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) = 0;
};
