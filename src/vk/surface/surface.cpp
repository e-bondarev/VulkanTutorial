#include "surface.h"

#include "../../window/window.h"
#include "../instance/instance.h"

namespace Vk {
namespace Surface {

VkSurfaceKHR surface = VK_NULL_HANDLE;

void Create()
{	
    VK_CHECK(glfwCreateWindowSurface(Instance::instance, Window::glfwWindow, nullptr, &surface), "Failed to create window surface.");
}

void Destroy()
{	
	vkDestroySurfaceKHR(Instance::instance, surface, nullptr);
}

}
}