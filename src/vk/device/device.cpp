#include "device.h"

#include "../instance/instance.h"
#include "../instance/validation.h"
#include "../surface/surface.h"
#include "../swap_chain/support_details.h"
#include "queue_family.h"

namespace Vk
{
	namespace Global
	{
		Device *device;

		Device::Device()
		{
			PickPhysicalDevice();
			CreateLogicalDevice();

			TRACE();
		}

		Device::~Device()
		{
			vkDestroyDevice(vkDevice, nullptr);

			TRACE();
		}

		bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const auto &extension : availableExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		void Device::PickPhysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance->GetVkInstance(), &deviceCount, nullptr);

			if (deviceCount == 0)
			{
				THROW("Failed to find GPUs with Vulkan support.");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance->GetVkInstance(), &deviceCount, devices.data());

			for (const auto &device : devices)
			{
				if (IsDeviceSuitable(device))
				{
					vkPhysicalDevice = device;
					break;
				}
			}

			if (vkPhysicalDevice == VK_NULL_HANDLE)
			{
				THROW("Failed to find a suitable GPU.");
			}
		}

		bool Device::IsDeviceSuitable(VkPhysicalDevice device)
		{
			Queues::indices = Queues::FindQueueFamilies(device);

			bool extensionsSupported = CheckDeviceExtensionSupport(device);
			bool swapChainAdequate = false;

			if (extensionsSupported)
			{
				SupportDetails swapChainSupportDetails = QuerySwapChainSupport(device);
				swapChainAdequate = !swapChainSupportDetails.formats.empty() && !swapChainSupportDetails.presentModes.empty();
			}

			return Queues::indices.IsComplete() && swapChainAdequate;
		}

		void Device::CreateLogicalDevice()
		{
			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies =
				{
					Queues::indices.graphicsFamily.value(), Queues::indices.presentFamily.value()};

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

			VK_CHECK(vkCreateDevice(vkPhysicalDevice, &createInfo, nullptr, &vkDevice), "Failed to create logical device.");

			vkGetDeviceQueue(vkDevice, Queues::indices.graphicsFamily.value(), 0, &Queues::graphicsQueue);
			vkGetDeviceQueue(vkDevice, Queues::indices.presentFamily.value(), 0, &Queues::presentQueue);
		}

		void Device::WaitIdle() const
		{
			vkDeviceWaitIdle(vkDevice);
		}

		VkPhysicalDevice Device::GetVkPhysicalDevice() const
		{
			return vkPhysicalDevice;
		}

		VkDevice Device::GetVkDevice() const
		{
			return vkDevice;
		}
	}
}