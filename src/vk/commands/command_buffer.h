#pragma once

#include "../../common.h"

#include "command_pool.h"

namespace Vk
{
	class CommandBuffer
	{
	public:
		CommandBuffer(CommandPool* command_pool);
		~CommandBuffer();

		void Begin() const;
		void End() const;

		VkCommandBuffer& GetVkCommandBuffer();

	private:
		VkCommandBuffer vkCommandBuffer;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}