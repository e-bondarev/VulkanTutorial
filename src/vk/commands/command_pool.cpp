#include "command_pool.h"

#include "../device/queue_family.h"
#include "../device/device.h"

namespace Vk
{
	CommandPool::CommandPool()
	{
		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = Vk::Queues::indices.graphicsFamily.value();
		pool_info.flags = 0; // Optional

		VK_CHECK(vkCreateCommandPool(device->GetVkDevice(), &pool_info, nullptr, &vkCommandPool), "Failed to create command pool.");

		TRACE();
	}

	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(device->GetVkDevice(), vkCommandPool, nullptr);

		TRACE();
	}

	void CommandPool::Reset() const
	{
        VK_CHECK(vkResetCommandPool(Vk::device->GetVkDevice(), vkCommandPool, 0), "Failed to reset command pool.");
	}

	VkCommandPool CommandPool::GetVkCommandPool() const
	{
		return vkCommandPool;
	}
}