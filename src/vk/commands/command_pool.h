#pragma once

#include "../../common.h"

namespace Vk
{
	class CommandPool
	{
	public:
		CommandPool();
		~CommandPool();

		VkCommandPool GetVkCommandPool() const;

	private:
		VkCommandPool vkCommandPool;

		CommandPool(const CommandPool&) = delete;
		CommandPool& operator=(const CommandPool&) = delete;
	};
}