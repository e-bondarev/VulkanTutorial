#include "descriptor_set_layout.h"

#include "../device/device.h"

namespace Vk
{
	DescriptorSetLayout::DescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding ubo_layout_binding{};
		
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ubo_layout_binding.pImmutableSamplers = nullptr; // Optional

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &ubo_layout_binding;

		VK_CHECK(vkCreateDescriptorSetLayout(Global::device->GetVkDevice(), &layout_info, nullptr, &vkDescriptorSetLayout), "Failed to create descriptor set layout.");
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(Global::device->GetVkDevice(), vkDescriptorSetLayout, nullptr);
	}

	VkDescriptorSetLayout& DescriptorSetLayout::GetVkDescriptorSetLayout()
	{
		return vkDescriptorSetLayout;
	}
}