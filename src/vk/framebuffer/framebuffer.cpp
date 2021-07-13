#include "framebuffer.h"

#include "../device/device.h"

namespace Vk
{
	Framebuffer::Framebuffer(VkImageView image_view, VkRenderPass render_pass, const glm::vec2& size)
	{
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = &image_view;
		framebuffer_info.width = static_cast<uint32_t>(size.x);
		framebuffer_info.height = static_cast<uint32_t>(size.y);
		framebuffer_info.layers = 1;

		VK_CHECK(vkCreateFramebuffer(device->GetVkDevice(), &framebuffer_info, nullptr, &vkFramebuffer), "Failed to create framebuffer.");

		TRACE();
	}

	Framebuffer::~Framebuffer()
	{
        vkDestroyFramebuffer(device->GetVkDevice(), vkFramebuffer, nullptr);

		TRACE();
	}

	VkFramebuffer Framebuffer::GetVkFramebuffer() const
	{
		return vkFramebuffer;
	}
}