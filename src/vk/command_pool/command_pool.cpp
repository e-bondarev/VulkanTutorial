#include "command_pool.h"

#include "../gpu/gpu.h"
#include "../gpu/queue_family.h"

namespace Vk {

CommandPool::CommandPool()
{	
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = Queues::indices.graphicsFamily.value();
	poolInfo.flags = 0; // Optional

	VK_CHECK(vkCreateCommandPool(GPU::gpu, &poolInfo, nullptr, &commandPool), "Failed to create command pool.");
}

CommandPool::~CommandPool()
{
    vkDestroyCommandPool(GPU::gpu, commandPool, nullptr);
}

const VkCommandPool& CommandPool::GetVkCommandPool() const
{
	return commandPool;
}

}