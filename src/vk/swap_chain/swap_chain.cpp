#include "swap_chain.h"

#include "../../window/window.h"
#include "support_details.h"
#include "../gpu/gpu.h"
#include "../gpu/queue_family.h"

namespace Vk {
namespace SwapChain {
	
VkSwapchainKHR swapChain = VK_NULL_HANDLE;

std::vector<VkImage> images;
VkFormat imageFormat;
VkExtent2D extent;

std::vector<VkImageView> imageViews;

VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) 
	{
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{	
    for (const auto& availablePresentMode : availablePresentModes) 
	{
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{	
    if (capabilities.currentExtent.width != UINT32_MAX) 
	{
        return capabilities.currentExtent;
    }
	else 
	{
        int width, height;
        glfwGetFramebufferSize(Window::glfwWindow, &width, &height);

        VkExtent2D actualExtent = 
		{
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void CreateImageViews()
{
	imageViews.resize(images.size());

	for (size_t i = 0; i < images.size(); i++) 
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(GPU::gpu, &createInfo, nullptr, &imageViews[i]), "Failed to create image views.");
	}
}

void DestroyImageViews()
{	
    for (const auto& imageView : imageViews) 
	{
        vkDestroyImageView(GPU::gpu, imageView, nullptr);
    }
}

void Create()
{
	SupportDetails supportDetails = QuerySwapChainSupport(GPU::physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(supportDetails.formats);
    VkPresentModeKHR presentMode = ChoosePresentMode(supportDetails.presentModes);
    extent = ChooseExtent(supportDetails.capabilities);

	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;

	if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) 
	{
    	imageCount = supportDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = Surface::surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	std::array<uint32_t, 2> queueFamilyIndices = 
	{
		Queues::indices.graphicsFamily.value(), Queues::indices.presentFamily.value()
	};

	if (Queues::indices.graphicsFamily != Queues::indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	} 
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = supportDetails.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(GPU::gpu, &createInfo, nullptr, &swapChain), "Failed to create swap chain.");

	vkGetSwapchainImagesKHR(GPU::gpu, swapChain, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(GPU::gpu, swapChain, &imageCount, images.data());

	imageFormat = surfaceFormat.format;

	CreateImageViews();
}

void Destroy()
{
	DestroyImageViews();

	vkDestroySwapchainKHR(GPU::gpu, swapChain, nullptr);
}

}
}