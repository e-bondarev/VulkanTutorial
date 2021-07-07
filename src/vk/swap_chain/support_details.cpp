#include "support_details.h"

namespace Vk {
namespace SwapChain {

SupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
{
	SupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, Surface::surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface::surface, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, Surface::surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, Surface::surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, Surface::surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

}
}