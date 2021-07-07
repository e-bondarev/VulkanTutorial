#include "gpu.h"

#include "../instance/instance.h"
#include "../instance/validation.h"

namespace Vk {
namespace GPU {

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

QueueFamilyIndices indices = {};

void PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance::instance, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
    	THROW("Failed to find GPUs with Vulkan support.");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance::instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (IsDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		THROW("Failed to find a suitable GPU.");
	}
}

bool IsDeviceSuitable(VkPhysicalDevice device) 
{
	indices = FindQueueFamilies(device);

    return indices.IsComplete();
}

bool QueueFamilyIndices::IsComplete() const
{
	return graphicsFamily.has_value();
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices newIndices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			newIndices.graphicsFamily = i;

			if (newIndices.IsComplete())
			{
				break;
			}
		}

		i++;
	}

	return newIndices;
}

VkDevice gpu = VK_NULL_HANDLE;

void CreateLogicalDevice()
{
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;

	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;

	if (Validation::enableValidationLayers) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(Validation::validationLayers.size());
		createInfo.ppEnabledLayerNames = Validation::validationLayers.data();
	} 
	else 
	{
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &gpu), "Failed to create logical device.");

	vkGetDeviceQueue(gpu, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

VkQueue graphicsQueue = VK_NULL_HANDLE;

void Create()
{
	PickPhysicalDevice();
	CreateLogicalDevice();
}

void Destroy()
{
	vkDestroyDevice(gpu, nullptr);
}

}
}