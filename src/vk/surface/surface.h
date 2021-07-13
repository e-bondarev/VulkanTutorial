#pragma once

#include "../../common.h"

namespace Vk
{
	class Surface
	{
	public:
		Surface();
		~Surface();

		VkSurfaceKHR GetVkSurface() const;

	private:
		VkSurfaceKHR vkSurface;
	};

	extern Surface* surface;
}