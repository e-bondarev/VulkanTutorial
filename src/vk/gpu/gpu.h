#pragma once

#include "../../common.h"

namespace Vk {
namespace GPU {

const std::vector<const char*> deviceExtensions = 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

extern VkPhysicalDevice physicalDevice;
void PickPhysicalDevice();
bool IsDeviceSuitable(VkPhysicalDevice device);

extern VkDevice gpu;
void CreateLogicalDevice();

void Create();
void Destroy();

}
}