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

		void Begin(VkCommandBufferUsageFlags flags = 0) const;
		void End() const;

		void SubmitToQueue(const VkQueue& queue, VkSemaphore* wait_semaphore = nullptr, const VkSemaphore* signal_semaphore = nullptr, VkFence fence = nullptr) const;

		VkCommandBuffer& GetVkCommandBuffer();

	private:
		VkCommandBuffer vkCommandBuffer;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}