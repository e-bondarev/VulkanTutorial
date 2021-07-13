#include "surface.h"

#include "../../window/window.h"
#include "../instance/instance.h"

namespace Vk
{
	Surface* surface;

	Surface::Surface()
	{
		VK_CHECK(glfwCreateWindowSurface(instance->GetVkInstance(), Window::glfwWindow, nullptr, &vkSurface), "Failed to create window surface.");

		TRACE();
	}

	Surface::~Surface()
	{
		vkDestroySurfaceKHR(instance->GetVkInstance(), vkSurface, nullptr);

		TRACE();
	}

	VkSurfaceKHR Surface::GetVkSurface() const
	{
		return vkSurface;
	}
}