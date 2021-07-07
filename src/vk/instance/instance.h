#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vk {
namespace Instance {

extern VkInstance instance;

void Create();
void Destroy();

}
}