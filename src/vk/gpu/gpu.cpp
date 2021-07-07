#include "gpu.h"

#include "../instance/instance.h"
#include "../instance/validation.h"
#include "../surface/surface.h"
#include "../swap_chain/support_details.h"
#include "queue_family.h"

namespace Vk {
namespace GPU {

VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

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
	Queues::indices = Queues::FindQueueFamilies(device);

	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainAdequate = false;

	if (extensionsSupported)
	{
		SwapChain::SupportDetails swapChainSupportDetails = SwapChain::QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
	}

    return Queues::indices.IsComplete() && swapChainAdequate;
}

VkDevice gpu = VK_NULL_HANDLE;

void CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = 
	{
		Queues::indices.graphicsFamily.value(), Queues::indices.presentFamily.value()
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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

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

	vkGetDeviceQueue(gpu, Queues::indices.graphicsFamily.value(), 0, &Queues::graphicsQueue);
	vkGetDeviceQueue(gpu, Queues::indices.presentFamily.value(), 0, &Queues::presentQueue);
}

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