#pragma once

#include "command_pool.h"

namespace Vk {
namespace DefaultCommandPool {

extern CommandPool* commandPool;

void Create();
void Destroy();

}
}