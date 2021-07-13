#pragma once

#include "../../common.h"

namespace Vk
{
	class DescriptorPool
	{
	public:
		DescriptorPool();
		~DescriptorPool();

		VkDescriptorPool& GetVkDescriptorPool();

	private:
		VkDescriptorPool vkDescriptorPool;

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
	};
}