#include "command_buffer.h"

#include "../device/device.h"

namespace Vk
{
	CommandBuffer::CommandBuffer(CommandPool* command_pool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = command_pool->GetVkCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(device->GetVkDevice(), &allocInfo, &vkCommandBuffer), "Failed to allocate command buffers.");

		TRACE();
	}

	CommandBuffer::~CommandBuffer()
	{
		TRACE();
	}

	void CommandBuffer::Begin() const
	{		
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		VK_CHECK(vkBeginCommandBuffer(vkCommandBuffer, &begin_info), "Failed to begin recording command buffer.");
	}

	void CommandBuffer::End() const
	{
		VK_CHECK(vkEndCommandBuffer(vkCommandBuffer), "Failed to record command buffer.");
	}

	VkCommandBuffer& CommandBuffer::GetVkCommandBuffer()
	{
		return vkCommandBuffer;
	}
}