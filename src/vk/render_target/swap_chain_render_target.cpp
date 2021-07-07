#include "swap_chain_render_target.h"

#include "../swap_chain/swap_chain.h"

namespace Vk {
namespace SwapChainRenderTarget {

RenderTarget* renderTarget;

void Create(VkRenderPass renderPass)
{
	renderTarget = new RenderTarget(SwapChain::extent, SwapChain::imageViews, renderPass);
}

void Destroy()
{
	delete renderTarget;
}

}
}