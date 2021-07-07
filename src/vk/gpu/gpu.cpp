#include "gpu.h"

#include "../instance/instance.h"
#include "../instance/validation.h"
#include "../surface/surface.h"

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
	return graphicsFamily.has_value() && presentFamily.has_value();
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

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, Surface::surface, &presentSupport);

            if (presentSupport) 
			{
                newIndices.presentFamily = i;
            }

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
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = 
	{
		indices.graphicsFamily.value(), indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) 
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

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