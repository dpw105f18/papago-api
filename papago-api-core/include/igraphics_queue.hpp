#pragma once
class ICommandBuffer;
class IImageResource;
class ISwapchain;

class IGraphicsQueue
{
public:
	virtual ~IGraphicsQueue() = default;

	virtual void present(ISwapchain& swapchain) = 0;
	virtual void submitCommands(const std::vector<std::reference_wrapper<ICommandBuffer>>&) = 0;
	virtual void submitPresent(const std::vector<std::reference_wrapper<ICommandBuffer>>&, ISwapchain&) = 0;
};
