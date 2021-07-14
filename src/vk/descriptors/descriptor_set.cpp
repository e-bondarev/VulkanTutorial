#include "descriptor_set.h"

#include "../device/device.h"

namespace Vk
{
	DescriptorSet::DescriptorSet(DescriptorPool* descriptor_pool, VkDescriptorSetLayout* layouts, uint32_t amount_of_layouts)
	{
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool->GetVkDescriptorPool();
        alloc_info.descriptorSetCount = amount_of_layouts;
        alloc_info.pSetLayouts = layouts;

        vkDescriptorSets.resize(amount_of_layouts);

        VK_CHECK(vkAllocateDescriptorSets(Global::device->GetVkDevice(), &alloc_info, vkDescriptorSets.data()), "Failed to allocate descriptor sets.");
	}

	DescriptorSet::~DescriptorSet()
	{

	}

	const std::vector<VkDescriptorSet>& DescriptorSet::GetVkDescriptorSets() const
	{
		return vkDescriptorSets;
	}
}