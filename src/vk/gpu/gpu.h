#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vk {
namespace GPU {

extern VkPhysicalDevice physicalDevice;
void PickPhysicalDevice();
bool IsDeviceSuitable(VkPhysicalDevice device);

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete() const;
};

extern QueueFamilyIndices indices;
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

extern VkDevice gpu;
void CreateLogicalDevice();

extern VkQueue graphicsQueue;

void Create();
void Destroy();

}
}