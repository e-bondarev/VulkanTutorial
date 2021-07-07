#pragma once

#include "../../common.h"

namespace Vk {

class Framebuffer
{
public:
	Framebuffer(VkExtent2D extent, VkImageView imageView, VkRenderPass renderPass);
	~Framebuffer();

	const VkFramebuffer& GetVkFramebuffer() const;

private:
	VkFramebuffer framebuffer;

	Framebuffer(const Framebuffer&);
	Framebuffer& operator=(const Framebuffer&);
};

}