#include "support_details.h"

namespace Vk {

SupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
{
	SupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface->GetVkSurface(), &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface->GetVkSurface(), &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface->GetVkSurface(), &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface->GetVkSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface->GetVkSurface(), &presentModeCount, details.presentModes.data());
	}

	return details;
}

}