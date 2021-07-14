#include "buffer.h"

#include "../device/device.h"

namespace Vk
{
	Buffer::Buffer(uint32_t size_of_element, uint32_t amount_of_elements, const void* data) : sizeOfElement { size_of_element }, amountOfElements { amount_of_elements }
	{
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size_of_element * amount_of_elements;
		buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(Global::device->GetVkDevice(), &buffer_info, nullptr, &vkBuffer), "Failed to create vertex buffer.");
		
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(Global::device->GetVkDevice(), vkBuffer, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = Global::device->FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VK_CHECK(vkAllocateMemory(Global::device->GetVkDevice(), &alloc_info, nullptr, &vkMemory), "Failed to allocate vertex buffer memory.");

        vkBindBufferMemory(Global::device->GetVkDevice(), vkBuffer, vkMemory, 0);

        void* mapped_data;
        vkMapMemory(Global::device->GetVkDevice(), vkMemory, 0, buffer_info.size, 0, &mapped_data);
            memcpy(mapped_data, data, static_cast<size_t>(buffer_info.size));
        vkUnmapMemory(Global::device->GetVkDevice(), vkMemory);
	}

	Buffer::~Buffer()
	{
		vkDestroyBuffer(Global::device->GetVkDevice(), vkBuffer, nullptr);
    	vkFreeMemory(Global::device->GetVkDevice(), vkMemory, nullptr);
	}

	VkBuffer& Buffer::GetVkBuffer()
	{
		return vkBuffer;
	}

	VkDeviceMemory& Buffer::GetVkMemory()
	{
		return vkMemory;
	}
}