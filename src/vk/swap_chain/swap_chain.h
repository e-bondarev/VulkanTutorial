#pragma once

#include "../../common.h"

namespace Vk
{
	class SwapChain
	{
	public:
		SwapChain();
		~SwapChain();

		VkSwapchainKHR GetVkSwapChain() const;
		VkFormat GetImageFormat() const;
		VkSurfaceFormatKHR GetSurfaceFormat() const;
		VkExtent2D GetExtent() const;

		const std::vector<VkImage>& GetImages() const;
		const std::vector<VkImageView>& GetImageViews() const;

	private:
		VkSwapchainKHR swapChain;

		VkFormat imageFormat;
		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR presentMode;
		VkExtent2D extent;

		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities);

		void CreateImageViews();
		void DestroyImageViews();
	};

	extern SwapChain* swapChain;
}