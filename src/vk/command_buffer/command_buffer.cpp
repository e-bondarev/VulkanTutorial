#include "command_buffer.h"

#include "../gpu/gpu.h"

namespace Vk {

CommandBuffer::CommandBuffer()
{	
}

CommandBuffer::~CommandBuffer()
{
}

const VkCommandBuffer& CommandBuffer::GetVkCommandBuffer() const
{
	return commandBuffer;
}

}