#pragma once

#include "../../common.h"

namespace Vk {

class CommandPool
{
public:
	CommandPool();
	~CommandPool();

	const VkCommandPool& GetVkCommandPool() const;

private:
	VkCommandPool commandPool;

	CommandPool(const CommandPool&) = delete;
	CommandPool& operator=(const CommandPool&) = delete;
};

}