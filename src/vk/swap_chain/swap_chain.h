#pragma once

#include "../../common.h"

namespace Vk {
namespace SwapChain {

extern VkSwapchainKHR swapChain;
extern std::vector<VkImage> images;

extern VkFormat imageFormat;
extern VkExtent2D extent;

VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

extern std::vector<VkImageView> imageViews;
void CreateImageViews();
void DestroyImageViews();

void Create();
void Destroy();

}
}