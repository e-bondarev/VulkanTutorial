#include "window/window.h"
#include "assets/assets.h"
#include "assets/text_asset.h"

#include "vk/instance/instance.h"

#include "vk/device/device.h"
#include "vk/device/queue_family.h"

#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

#include "vk/pipeline/pipeline.h"
#include "vk/framebuffer/framebuffer.h"
#include "vk/commands/command_pool.h"
#include "vk/commands/command_buffer.h"

#include "vk/descriptors/descriptor_pool.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

Vk::Pipeline* pipeline;
std::vector<Vk::Framebuffer*> framebuffers;
std::vector<Vk::CommandPool*> commandPools;
std::vector<Vk::CommandBuffer*> commandBuffers;

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct Frame
{
	VkSemaphore ImageAvailable;
	VkSemaphore RenderFinished;
	VkFence InFlightFence;
};

std::vector<Frame> frames;
std::vector<VkFence> imagesInFlight;

int currentFrame = 0;

Vk::DescriptorPool* descriptorPool;

void Window::OnInit()
{
	Vk::instance = new Vk::Instance();
	Vk::surface = new Vk::Surface();
	Vk::device = new Vk::Device();
	Vk::swapChain = new Vk::SwapChain();

	glm::vec2 viewport_size = { Vk::swapChain->GetExtent().width, Vk::swapChain->GetExtent().height };

	Assets::Text vs_code("assets/shaders/default.vert.spv");
	Assets::Text fs_code("assets/shaders/default.frag.spv");
	
	pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), viewport_size, Vk::swapChain->GetImageFormat());

	for (const VkImageView& image_view : Vk::swapChain->GetImageViews())
	{
		framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));
	}

	for (const VkImageView& image_view : Vk::swapChain->GetImageViews())
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
			renderPassInfo.renderArea.extent = Vk::swapChain->GetExtent();

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
		VK_CHECK(vkCreateSemaphore(Vk::device->GetVkDevice(), &semaphore_info, nullptr, &frames[i].ImageAvailable), "Failed to create semaphore 1");
		VK_CHECK(vkCreateSemaphore(Vk::device->GetVkDevice(), &semaphore_info, nullptr, &frames[i].RenderFinished), "Failed to create semaphore 2");
		VK_CHECK(vkCreateFence(Vk::device->GetVkDevice(), &fence_info, nullptr, &frames[i].InFlightFence), "Failed to create fence 1");
	}

	descriptorPool = new Vk::DescriptorPool();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(Window::glfwWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Vk::instance->GetVkInstance();
    init_info.PhysicalDevice = Vk::device->GetVkPhysicalDevice();
    init_info.Device = Vk::device->GetVkDevice();
    init_info.QueueFamily = Vk::Queues::indices.graphicsFamily.value();
    init_info.Queue = Vk::Queues::graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = descriptorPool->GetVkDescriptorPool();
    init_info.Allocator = nullptr;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, pipeline->GetRenderPass()->GetVkRenderPass());

	
    {
		Vk::CommandPool* my_command_pool = new Vk::CommandPool();
		Vk::CommandBuffer* my_command_buffer = new Vk::CommandBuffer(my_command_pool);

        VK_CHECK(vkResetCommandPool(Vk::device->GetVkDevice(), my_command_pool->GetVkCommandPool(), 0), "Failed to reset command pool.");
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(my_command_buffer->GetVkCommandBuffer(), &begin_info), "Failed to begin command buffer.");

        ImGui_ImplVulkan_CreateFontsTexture(my_command_buffer->GetVkCommandBuffer());

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &my_command_buffer->GetVkCommandBuffer();
        VK_CHECK(vkEndCommandBuffer(my_command_buffer->GetVkCommandBuffer()), "Failed to end command buffer.");
        VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &end_info, VK_NULL_HANDLE), "Failed to submit queue.");

        vkDeviceWaitIdle(Vk::device->GetVkDevice());
        ImGui_ImplVulkan_DestroyFontUploadObjects();

		delete my_command_buffer;
		delete my_command_pool;
    }
}

void Render(uint32_t& image_index)
{
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &frames[currentFrame].ImageAvailable;
	submit_info.pWaitDstStageMask = waitStages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &commandBuffers[image_index]->GetVkCommandBuffer();	

	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &frames[currentFrame].RenderFinished;

	vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);
	VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &submit_info, frames[currentFrame].InFlightFence), "Failed to submit draw command buffer.");
}

void Present(uint32_t& image_index)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &frames[currentFrame].RenderFinished;

	VkSwapchainKHR swapChains[] = { Vk::swapChain->GetVkSwapChain() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr; // Optional

	VK_CHECK(vkQueuePresentKHR(Vk::Queues::presentQueue, &present_info), "Failed to present.");
}

void DrawFrame()
{	
    uint32_t imageIndex;
    vkAcquireNextImageKHR(Vk::device->GetVkDevice(), Vk::swapChain->GetVkSwapChain(), UINT64_MAX, frames[currentFrame].ImageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
        vkWaitForFences(Vk::device->GetVkDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

		ImGui::ShowDemoWindow();
	
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

    vkWaitForFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);

    imagesInFlight[imageIndex] = frames[currentFrame].InFlightFence;

    {
        VK_CHECK(vkResetCommandPool(Vk::device->GetVkDevice(), commandPools[currentFrame]->GetVkCommandPool(), 0), "Failed to reset command pool.");
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(commandBuffers[currentFrame]->GetVkCommandBuffer(), &info), "Failed to begin command buffer.");
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = pipeline->GetRenderPass()->GetVkRenderPass();
        info.framebuffer = framebuffers[imageIndex]->GetVkFramebuffer();
        info.renderArea.extent = Vk::swapChain->GetExtent();
        info.clearValueCount = 1;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        info.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffers[currentFrame]->GetVkCommandBuffer(), &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[currentFrame]->GetVkCommandBuffer());

    vkCmdEndRenderPass(commandBuffers[currentFrame]->GetVkCommandBuffer());
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &frames[currentFrame].ImageAvailable;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffers[currentFrame]->GetVkCommandBuffer();
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &frames[currentFrame].RenderFinished;

        VK_CHECK(vkEndCommandBuffer(commandBuffers[currentFrame]->GetVkCommandBuffer()), "Failed to end command buffer.");
        VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &info, frames[currentFrame].InFlightFence), "Failed to submit queue.");
    }

	// Render(imageIndex);
	Present(imageIndex);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Window::OnUpdate()
{
	glfwPollEvents();

	DrawFrame();
}

void Window::OnShutdown()
{
	vkDeviceWaitIdle(Vk::device->GetVkDevice());
	
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete descriptorPool;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(Vk::device->GetVkDevice(), frames[i].ImageAvailable, nullptr);
		vkDestroySemaphore(Vk::device->GetVkDevice(), frames[i].RenderFinished, nullptr);
        vkDestroyFence(Vk::device->GetVkDevice(), frames[i].InFlightFence, nullptr);
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

	delete Vk::swapChain;
	delete Vk::device;
	delete Vk::surface;
	delete Vk::instance;
}

int main(int amountOfArguments, char *arguments[])
{
	Assets::LocateRoot(amountOfArguments, arguments);

	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}