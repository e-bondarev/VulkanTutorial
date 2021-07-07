#include "swap_chain_command_buffers.h"

#include "../render_target/swap_chain_render_target.h"

namespace Vk {
namespace SwapChainCommandBuffers {
	
std::vector<CommandBuffer*> commandBuffers;

void Create()
{
	for (int i = 0; i < SwapChainRenderTarget::renderTarget->GetFramebuffers().size(); i++)
	{
		commandBuffers.push_back(new CommandBuffer());
	}
}

void Destroy()
{
	for (auto& buffer : commandBuffers)
	{
		delete buffer;
	}
}

}
}