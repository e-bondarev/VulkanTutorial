#include "framebuffer.h"

#include "../gpu/gpu.h"

namespace Vk {

Framebuffer::Framebuffer(VkExtent2D extent, VkImageView imageView, VkRenderPass renderPass)
{
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;

    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &imageView;

    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    VK_CHECK(vkCreateFramebuffer(GPU::gpu, &framebufferInfo, nullptr, &framebuffer), "Failed to create framebuffer.");
}

Framebuffer::~Framebuffer()
{
	vkDestroyFramebuffer(GPU::gpu, framebuffer, nullptr);
}

}