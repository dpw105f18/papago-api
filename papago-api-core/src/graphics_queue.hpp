#pragma once
#include <vector>

class CommandBuffer;
class GraphicsQueue
{
public:
	void submitCommands(std::vector<CommandBuffer>);
};