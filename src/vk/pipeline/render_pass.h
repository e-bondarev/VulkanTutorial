#pragma once

#include "../../common.h"

#include "../framebuffer/framebuffer.h"
#include "../commands/command_buffer.h"

namespace Vk
{
	class RenderPass
	{
	public:
		RenderPass(VkFormat format);
		~RenderPass();

		VkRenderPass GetVkRenderPass() const;

		void Begin(CommandBuffer* command_buffer, Framebuffer* framebuffer) const;
		void End(CommandBuffer* command_buffer) const;

	private:
		VkRenderPass vkRenderPass;

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
	};
}