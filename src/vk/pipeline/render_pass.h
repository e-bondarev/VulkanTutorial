#pragma once

#include "../../common.h"

namespace Vk {

class RenderPass
{
friend class Pipeline;

public:
	RenderPass();
	~RenderPass();

	VkRenderPass GetVkRenderPass() const;

private:
	VkRenderPass renderPass;

	RenderPass(const RenderPass&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;
};

}