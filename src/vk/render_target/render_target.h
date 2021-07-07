#pragma once

#include "../../common.h"

#include "../framebuffer/framebuffer.h"

namespace Vk {

class RenderTarget
{
public:
	RenderTarget(VkExtent2D extent, const std::vector<VkImageView>& imageViews, VkRenderPass renderPass);
	~RenderTarget();

	const std::vector<std::shared_ptr<Framebuffer>>& GetFramebuffers() const;

private:
	std::vector<std::shared_ptr<Framebuffer>> framebuffers;

	RenderTarget(const RenderTarget&);
	RenderTarget& operator=(const RenderTarget&);
};

}