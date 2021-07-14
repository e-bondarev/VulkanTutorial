#include "triangle.h"

#include "../../assets/text_asset.h"
#include "../../window/window.h"

#include "../../vk/instance/instance.h"
#include "../../vk/surface/surface.h"
#include "../../vk/device/device.h"
#include "../../vk/device/queue_family.h"
#include "../../vk/swap_chain/swap_chain.h"

namespace Examples
{
	Triangle::Triangle()
	{
		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/default.vert.spv");
		Assets::Text fs_code("assets/shaders/default.frag.spv");
		
		pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), viewport_size, Vk::Global::swapChain->GetImageFormat());

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			commandPools.push_back(new Vk::CommandPool());

		for (Vk::CommandPool* command_pool : commandPools)
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);

		frameManager = new Vk::FrameManager();
	}

	void Triangle::RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer)
	{	
		command_pool->Reset();
			command_buffer->Begin();
				command_buffer->BeginRenderPass(pipeline->GetRenderPass(), framebuffer);
					command_buffer->BindPipeline(pipeline);
					command_buffer->Draw(3, 1, 0, 0);
				command_buffer->EndRenderPass();
			command_buffer->End();
	}

	void Triangle::Draw(Vk::CommandBuffer* command_buffer)
	{
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		command_buffer->SubmitToQueue(
			Vk::Global::Queues::graphicsQueue, 
			&current_frame->GetImageAvailableSemaphore(), 
			&current_frame->GetRenderFinishedSemaphore(), 
			current_frame->GetInFlightFence()
		);
	}

	void Triangle::Present()
	{
		VkResult result = Vk::Global::swapChain->Present(&frameManager->GetCurrentFrame()->GetRenderFinishedSemaphore(), 1);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		{

		} 
		else if (result != VK_SUCCESS) 
		{
			THROW("Can't present.");
		}

		frameManager->NextFrame();
	}

	void Triangle::Render()
	{		
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		uint32_t image_index = Vk::Global::swapChain->AcquireImage(current_frame->GetImageAvailableSemaphore());

		if (imagesInFlight[image_index] != VK_NULL_HANDLE) 
			vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &imagesInFlight[image_index], VK_TRUE, UINT64_MAX);

		vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence(), VK_TRUE, UINT64_MAX);
		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		imagesInFlight[image_index] = current_frame->GetInFlightFence();

		Vk::CommandPool* current_command_pool = commandPools[image_index];
		Vk::CommandBuffer* current_command_buffer = commandBuffers[image_index];	
		Vk::Framebuffer* current_framebuffer = framebuffers[image_index];

		RecordCommandBuffer(current_command_pool, current_command_buffer, current_framebuffer);
		Draw(current_command_buffer);
		Present();
	}

	Triangle::~Triangle()
	{
		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());

		delete frameManager;

		for (const Vk::CommandPool* command_pool : commandPools)
		{
			delete command_pool;
		}

		for (const Vk::Framebuffer* framebuffer : framebuffers)
		{
			delete framebuffer;
		}

		delete pipeline;
	}

	void Triangle::BeforeResize()
	{
		for (const Vk::Framebuffer* framebuffer : framebuffers)
			delete framebuffer;
			
		for (const Vk::CommandBuffer* command_buffer : commandBuffers)
			delete command_buffer;

		framebuffers.clear();
		commandBuffers.clear();

		imagesInFlight.clear();

		delete pipeline;
	}

	void Triangle::AfterResize()
	{
		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/default.vert.spv");
		Assets::Text fs_code("assets/shaders/default.frag.spv");
		
		pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), viewport_size, Vk::Global::swapChain->GetImageFormat());

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));

		for (Vk::CommandPool* command_pool : commandPools)
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);
	}
}