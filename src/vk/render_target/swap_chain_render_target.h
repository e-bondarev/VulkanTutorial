#pragma once

#include "render_target.h"

namespace Vk {
namespace SwapChainRenderTarget {

extern RenderTarget* renderTarget;

void Create(VkRenderPass renderPass);
void Destroy();

}
}