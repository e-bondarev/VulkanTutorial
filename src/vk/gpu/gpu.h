#pragma once

#include "../../common.h"

namespace Vk {
namespace GPU {

extern VkPhysicalDevice physicalDevice;
void PickPhysicalDevice();
bool IsDeviceSuitable(VkPhysicalDevice device);

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const;
};

extern QueueFamilyIndices indices;
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

extern VkDevice gpu;
void CreateLogicalDevice();

extern VkQueue graphicsQueue;
extern VkQueue presentQueue;

void Create();
void Destroy();

}
}