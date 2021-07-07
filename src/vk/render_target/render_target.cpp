#include "render_target.h"

namespace Vk {

RenderTarget::RenderTarget(VkExtent2D extent, const std::vector<VkImageView>& imageViews, VkRenderPass renderPass)
{
	for (int i = 0; i < imageViews.size(); i++)
	{
		framebuffers.emplace_back(std::make_shared<Framebuffer>(extent, imageViews[i], renderPass));
	}
}

RenderTarget::~RenderTarget()
{
	
}

const std::vector<std::shared_ptr<Framebuffer>>& RenderTarget::GetFramebuffers() const
{
	return framebuffers;
}

}