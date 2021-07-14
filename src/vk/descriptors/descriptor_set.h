#pragma once

#include "../../common.h"

#include "descriptor_pool.h"

#include "../memory/buffer.h"

namespace Vk
{
	class DescriptorSet
	{
	public:
		DescriptorSet(DescriptorPool* descriptor_pool, VkDescriptorSetLayout* layouts);
		~DescriptorSet();

		void Update(Buffer* buffer, uint32_t range);

		VkDescriptorSet& GetVkDescriptorSet();
        VkDescriptorSetAllocateInfo alloc_info{};

	private:
		VkDescriptorSet vkDescriptorSet;

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}