#pragma once

#include "../../common.h"

namespace Vk
{
	class Framebuffer
	{
	public:
		Framebuffer(VkImageView image_view, VkRenderPass render_pass, const glm::vec2& size);
		~Framebuffer();

		VkFramebuffer GetVkFramebuffer() const;

	private:
		VkFramebuffer vkFramebuffer;

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;
	};
}