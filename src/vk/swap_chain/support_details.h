#pragma once

#include "../../common.h"

#include "../surface/surface.h"

namespace Vk {
namespace SwapChain {

struct SupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

}
}