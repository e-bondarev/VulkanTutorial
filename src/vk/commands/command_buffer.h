#pragma once

#include "../../common.h"

#include "command_pool.h"
#include "../framebuffer/framebuffer.h"

namespace Vk
{
	class RenderPass;
	class Pipeline;

	class CommandBuffer
	{
	public:
		CommandBuffer(CommandPool* command_pool);
		~CommandBuffer();

		void Begin(VkCommandBufferUsageFlags flags = 0) const;
		void End() const;

		void BeginRenderPass(const RenderPass* render_pass, Framebuffer* framebuffer) const;
		void EndRenderPass() const;

		void BindPipeline(const Pipeline* pipeline) const;

		void SubmitToQueue(const VkQueue& queue, VkSemaphore* wait_semaphore = nullptr, const VkSemaphore* signal_semaphore = nullptr, VkFence fence = nullptr) const;

		// void BindPipeline(const Pipeline* pipeline) const;

		template <typename... Args>
		void Draw(Args&&... args) const
		{
			vkCmdDraw(vkCommandBuffer, std::forward<Args>(args)...);
		}

		VkCommandBuffer& GetVkCommandBuffer();

	private:
		VkCommandBuffer vkCommandBuffer;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
	};
}