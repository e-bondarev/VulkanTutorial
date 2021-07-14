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

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);
		frameManager = new Vk::FrameManager();

		InitImGui();
	}

	void ImGUI::RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer)
	{
		command_pool->Reset();
			command_buffer->Begin();
				command_buffer->BeginRenderPass(pipeline->GetRenderPass(), framebuffer);
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer->GetVkCommandBuffer());
				command_buffer->EndRenderPass();
			command_buffer->End();
	}

	void ImGUI::Draw(Vk::CommandBuffer* command_buffer)
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

	void ImGUI::Present()
	{
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();
		Vk::Global::swapChain->Present(&current_frame->GetRenderFinishedSemaphore(), 1);
		frameManager->NextFrame();
	}

	void ImGUI::Render()
	{			
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
			ImGui::ShowDemoWindow();
		ImGui::Render();

		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		Vk::Global::swapChain->AcquireImage(current_frame->GetImageAvailableSemaphore());

		if (imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()] != VK_NULL_HANDLE) 
			vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()], VK_TRUE, UINT64_MAX);

		vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence(), VK_TRUE, UINT64_MAX);
		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		imagesInFlight[Vk::Global::swapChain->GetCurrentImageIndex()] = current_frame->GetInFlightFence();

		Vk::CommandPool* current_command_pool = commandPools[Vk::Global::swapChain->GetCurrentImageIndex()];
		Vk::CommandBuffer* current_command_buffer = commandBuffers[Vk::Global::swapChain->GetCurrentImageIndex()];	
		Vk::Framebuffer* current_framebuffer = framebuffers[Vk::Global::swapChain->GetCurrentImageIndex()];
		
		RecordCommandBuffer(current_command_pool, current_command_buffer, current_framebuffer);
		Draw(current_command_buffer);
		Present();
	}

	ImGUI::~ImGUI()
	{
		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());

		ShutdownImGui();

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
}