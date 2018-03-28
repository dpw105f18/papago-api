#include "standard_header.hpp"
#include "command_buffer.hpp"

CommandBuffer::operator vk::CommandBuffer&()
{
	return *m_vkCommandBuffer;
}
