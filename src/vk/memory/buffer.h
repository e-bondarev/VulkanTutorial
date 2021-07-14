#pragma once

#include "../../common.h"

namespace Vk
{
	class Buffer
	{
	public:
		Buffer(uint32_t size_of_element, uint32_t amount_of_elements = 0, const void* data = nullptr);
		~Buffer();

		VkBuffer& GetVkBuffer();
		VkDeviceMemory& GetVkMemory();

	private:
		VkBuffer vkBuffer;
		VkDeviceMemory vkMemory;

		uint32_t sizeOfElement;
		uint32_t amountOfElements;

		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
	};
}