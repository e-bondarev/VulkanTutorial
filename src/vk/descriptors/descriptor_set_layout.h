#pragma once

#include "../../common.h"

namespace Vk
{
	class DescriptorSetLayout
	{
	public:
		DescriptorSetLayout();
		~DescriptorSetLayout();

		VkDescriptorSetLayout& GetVkDescriptorSetLayout();

	private:
		VkDescriptorSetLayout vkDescriptorSetLayout;

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
	};
}