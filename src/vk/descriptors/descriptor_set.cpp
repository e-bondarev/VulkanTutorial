#include "descriptor_set.h"

#include "../device/device.h"

namespace Vk
{
	DescriptorSet::DescriptorSet(DescriptorPool* descriptor_pool, VkDescriptorSetLayout* layouts)
	{
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool->GetVkDescriptorPool();
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = layouts;

        VK_CHECK(vkAllocateDescriptorSets(Global::device->GetVkDevice(), &alloc_info, &vkDescriptorSet), "Failed to allocate descriptor sets.");
	}

	DescriptorSet::~DescriptorSet()
	{
	}

	void DescriptorSet::Update(Buffer* buffer, uint32_t range)
	{
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = buffer->GetVkBuffer();
		buffer_info.offset = 0;
		buffer_info.range = range;

		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = vkDescriptorSet;
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(Vk::Global::device->GetVkDevice(), 1, &descriptor_write, 0, nullptr);
	}

	VkDescriptorSet& DescriptorSet::GetVkDescriptorSet()
	{
		return vkDescriptorSet;
	}
}