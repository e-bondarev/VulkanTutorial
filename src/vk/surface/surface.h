#pragma once

#include "../../common.h"

namespace Vk
{
	namespace Global
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

		extern Surface *surface;
	}
}