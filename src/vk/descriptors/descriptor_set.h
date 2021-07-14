#pragma once

#include "../../common.h"

#include "descriptor_pool.h"

namespace Vk
{
	class DescriptorSet
	{
	public:
		DescriptorSet(DescriptorPool* descriptor_pool, VkDescriptorSetLayout* layouts, uint32_t amount_of_layouts);
		~DescriptorSet();

		const std::vector<VkDescriptorSet>& GetVkDescriptorSets() const;

	private:
		std::vector<VkDescriptorSet> vkDescriptorSets;

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
	};
}