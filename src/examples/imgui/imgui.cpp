#include "imgui.h"

#include "../../assets/text_asset.h"
#include "../../window/window.h"

#include "../../vk/instance/instance.h"
#include "../../vk/surface/surface.h"
#include "../../vk/device/device.h"
#include "../../vk/device/queue_family.h"
#include "../../vk/swap_chain/swap_chain.h"

namespace Examples
{
	void ImGUI::InitImGui()
	{		
		descriptorPool = new Vk::DescriptorPool();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForVulkan(Window::glfwWindow, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = Vk::Global::instance->GetVkInstance();
		init_info.PhysicalDevice = Vk::Global::device->GetVkPhysicalDevice();
		init_info.Device = Vk::Global::device->GetVkDevice();
		init_info.QueueFamily = Vk::Global::Queues::indices.graphicsFamily.value();
		init_info.Queue = Vk::Global::Queues::graphicsQueue;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool->GetVkDescriptorPool();
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = 2;
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info, pipeline->GetRenderPass()->GetVkRenderPass());

		Vk::CommandPool my_command_pool;
		Vk::CommandBuffer my_command_buffer(&my_command_pool);

		my_command_buffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			ImGui_ImplVulkan_CreateFontsTexture(my_command_buffer.GetVkCommandBuffer());
		my_command_buffer.End();

		my_command_buffer.SubmitToQueue(Vk::Global::Queues::graphicsQueue);

		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void ImGUI::ShutdownImGui()
	{		
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		delete descriptorPool;
	}

	ImGUI::ImGUI()
	{
		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/default.vert.spv");
		Assets::Text fs_code("assets/shaders/default.frag.spv");
		
		pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), viewport_size, Vk::Global::swapChain->GetImageFormat());

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
		{
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));
		}

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
		{
			commandPools.push_back(new Vk::CommandPool());
		}

		for (Vk::CommandPool* command_pool : commandPools)
		{
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));
		}

		for (int i = 0; i < commandBuffers.size(); i++)
		{
			commandBuffers[i]->Begin();
				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = pipeline->GetRenderPass()->GetVkRenderPass();
				renderPassInfo.framebuffer = framebuffers[i]->GetVkFramebuffer();

				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = Vk::Global::swapChain->GetExtent();

				VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(commandBuffers[i]->GetVkCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
					vkCmdBindPipeline(commandBuffers[i]->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());
					vkCmdDraw(commandBuffers[i]->GetVkCommandBuffer(), 3, 1, 0, 0);
				vkCmdEndRenderPass(commandBuffers[i]->GetVkCommandBuffer());		
			commandBuffers[i]->End();
		}

		frames.resize(MAX_FRAMES_IN_FLIGHT);
		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK(vkCreateSemaphore(Vk::Global::device->GetVkDevice(), &semaphore_info, nullptr, &frames[i].ImageAvailable), "Failed to create semaphore 1");
			VK_CHECK(vkCreateSemaphore(Vk::Global::device->GetVkDevice(), &semaphore_info, nullptr, &frames[i].RenderFinished), "Failed to create semaphore 2");
			VK_CHECK(vkCreateFence(Vk::Global::device->GetVkDevice(), &fence_info, nullptr, &frames[i].InFlightFence), "Failed to create fence 1");
		}

		InitImGui();
	}

	void ImGUI::Render()
	{			
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
			ImGui::ShowDemoWindow();
		ImGui::Render();

		Vk::Global::swapChain->AcquireImage(frames[currentFrame].ImageAvailable);

		if (imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()] != VK_NULL_HANDLE) 
			vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()], VK_TRUE, UINT64_MAX);

		vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);

		imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()] = frames[currentFrame].InFlightFence;

		Vk::CommandPool* current_command_pool = commandPools[currentFrame];
		Vk::CommandBuffer* current_command_buffer = commandBuffers[currentFrame];	
		Vk::Framebuffer* current_framebuffer = framebuffers[Vk::Global::swapChain->GetCurrentImageIndex()];

		current_command_pool->Reset();
		current_command_buffer->Begin();
			pipeline->GetRenderPass()->Begin(current_command_buffer, current_framebuffer);
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), current_command_buffer->GetVkCommandBuffer());
			pipeline->GetRenderPass()->End(current_command_buffer);
		current_command_buffer->End();

		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);
		
		current_command_buffer->SubmitToQueue(
			Vk::Global::Queues::graphicsQueue, 
			&frames[currentFrame].ImageAvailable, 
			&frames[currentFrame].RenderFinished, 
			frames[currentFrame].InFlightFence
		);	

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &frames[currentFrame].RenderFinished;

		VkSwapchainKHR swapChains[] = { Vk::Global::swapChain->GetVkSwapChain() };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapChains;
		uint32_t image_index = Vk::Global::swapChain->GetCurrentImageIndex();
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr; // Optional

		VK_CHECK(vkQueuePresentKHR(Vk::Global::Queues::presentQueue, &present_info), "Failed to present.");

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	ImGUI::~ImGUI()
	{
		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());

		ShutdownImGui();

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(Vk::Global::device->GetVkDevice(), frames[i].ImageAvailable, nullptr);
			vkDestroySemaphore(Vk::Global::device->GetVkDevice(), frames[i].RenderFinished, nullptr);
			vkDestroyFence(Vk::Global::device->GetVkDevice(), frames[i].InFlightFence, nullptr);
		}

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
}