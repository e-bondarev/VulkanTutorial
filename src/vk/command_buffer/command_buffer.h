#pragma once

#include "../../common.h"

namespace Vk {

class CommandBuffer
{
public:
	CommandBuffer();
	~CommandBuffer();

	const VkCommandBuffer& GetVkCommandBuffer() const;

private:
	VkCommandBuffer commandBuffer;

	CommandBuffer(const CommandBuffer&) = delete;
	CommandBuffer& operator=(const CommandBuffer&) = delete;
};

}