#include "command_buffer.h"

#include "../device/device.h"

#include "../pipeline/render_pass.h"

namespace Vk
{
	CommandBuffer::CommandBuffer(CommandPool* command_pool)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = command_pool->GetVkCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(Global::device->GetVkDevice(), &allocInfo, &vkCommandBuffer), "Failed to allocate command buffers.");

		TRACE();
	}

	CommandBuffer::~CommandBuffer()
	{
		TRACE();
	}

	void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) const
	{		
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = flags; // Optional
		begin_info.pInheritanceInfo = nullptr; // Optional

		VK_CHECK(vkBeginCommandBuffer(vkCommandBuffer, &begin_info), "Failed to begin recording command buffer.");
	}

	void CommandBuffer::End() const
	{
		VK_CHECK(vkEndCommandBuffer(vkCommandBuffer), "Failed to record command buffer.");
	}

	void CommandBuffer::SubmitToQueue(const VkQueue& queue, VkSemaphore* wait_semaphore, const VkSemaphore* signal_semaphore, VkFence fence) const
	{
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &vkCommandBuffer;

		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		if (wait_semaphore)
		{
        	submit_info.waitSemaphoreCount = 1;
        	submit_info.pWaitSemaphores = wait_semaphore;
        	submit_info.pWaitDstStageMask = &wait_stage;			
		}

		if (signal_semaphore)
		{
			submit_info.signalSemaphoreCount = 1;
			submit_info.pSignalSemaphores = signal_semaphore;
		}

        VK_CHECK(vkQueueSubmit(queue, 1, &submit_info, fence), "Failed to submit queue.");
	}

	void CommandBuffer::BeginRenderPass(const RenderPass* render_pass, Framebuffer* framebuffer) const
	{		
        VkRenderPassBeginInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        submit_info.renderPass = render_pass->GetVkRenderPass();
        submit_info.framebuffer = framebuffer->GetVkFramebuffer();
        submit_info.renderArea.extent = { static_cast<uint32_t>(framebuffer->GetSize().x), static_cast<uint32_t>(framebuffer->GetSize().y) };
        submit_info.clearValueCount = 1;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        submit_info.pClearValues = &clearColor;
        vkCmdBeginRenderPass(vkCommandBuffer, &submit_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void CommandBuffer::EndRenderPass() const
	{
    	vkCmdEndRenderPass(vkCommandBuffer);
	}

	// void CommandBuffer::BindPipeline(const Pipeline* pipeline) const
	// {
	// 	vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());
	// }

	VkCommandBuffer& CommandBuffer::GetVkCommandBuffer()
	{
		return vkCommandBuffer;
	}
}