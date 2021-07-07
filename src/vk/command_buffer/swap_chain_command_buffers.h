#pragma once

#include "command_buffer.h"

namespace Vk {
namespace SwapChainCommandBuffers {
	
extern std::vector<CommandBuffer*> commandBuffers;

void Create();
void Destroy();

}
}