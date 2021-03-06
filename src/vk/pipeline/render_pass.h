#pragma once

#include "../../common.h"

#include "../framebuffer/framebuffer.h"

namespace Vk
{
	class RenderPass
	{
	public:
		RenderPass(VkFormat format);
		~RenderPass();

		VkRenderPass GetVkRenderPass() const;

	private:
		VkRenderPass vkRenderPass;

		RenderPass(const RenderPass&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
	};
}