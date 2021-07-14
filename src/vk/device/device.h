#pragma once

#include "../../common.h"

namespace Vk
{
	namespace Global
	{
		class Device
		{
		public:
			Device();
			~Device();

			const std::vector<const char *> deviceExtensions =
				{
					VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

			void PickPhysicalDevice();
			bool IsDeviceSuitable(VkPhysicalDevice device);

			void CreateLogicalDevice();

			void WaitIdle() const;

			VkPhysicalDevice GetVkPhysicalDevice() const;
			VkDevice GetVkDevice() const;

		private:
			VkPhysicalDevice vkPhysicalDevice;
			VkDevice vkDevice;

			Device(const Device &) = delete;
			Device &operator=(const Device &) = delete;
		};

		extern Device *device;

	}
}