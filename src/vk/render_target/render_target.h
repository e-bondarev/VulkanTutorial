#pragma once

#include "../../common.h"

#include "../framebuffer/framebuffer.h"

namespace Vk {

class RenderTarget
{
public:
	RenderTarget(VkExtent2D extent, const std::vector<VkImageView>& imageViews, VkRenderPass renderPass);
	~RenderTarget();

private:
	std::vector<std::shared_ptr<Framebuffer>> framebuffers;

	RenderTarget(const RenderTarget&);
	RenderTarget& operator=(const RenderTarget&);
};

}