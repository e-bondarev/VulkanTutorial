#include "window/window.h"
#include "assets/assets.h"

#include "vk/instance/instance.h"

#include "vk/gpu/gpu.h"
#include "vk/gpu/queue_family.h"

#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

#include "vk/pipeline/default_pipeline.h"
#include "vk/render_target/render_target.h"
#include "vk/render_target/swap_chain_render_target.h"

#include "vk/command_pool/default_command_pool.h"
#include "vk/command_buffer/swap_chain_command_buffers.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

static VkInstance               g_Instance = VK_NULL_HANDLE;
static VkPhysicalDevice         g_PhysicalDevice = VK_NULL_HANDLE;
static VkDevice                 g_Device = VK_NULL_HANDLE;
static uint32_t                 g_QueueFamily = (uint32_t)-1;
static VkQueue                  g_Queue = VK_NULL_HANDLE;

static VkAllocationCallbacks*   g_Allocator = NULL;
static VkDebugReportCallbackEXT g_DebugReport = VK_NULL_HANDLE;
static VkPipelineCache          g_PipelineCache = VK_NULL_HANDLE;
static VkDescriptorPool         g_DescriptorPool = VK_NULL_HANDLE;

static ImGui_ImplVulkanH_Window g_MainWindowData;
static int                      g_MinImageCount = 2;
// static bool                     g_SwapChainRebuild = false;

std::vector<VkCommandBuffer> commandBuffers;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
// size_t currentFrame = 0;

void CreateCommandBuffers()
{
    commandBuffers.resize(Vk::SwapChainRenderTarget::renderTarget->GetFramebuffers().size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = Vk::DefaultCommandPool::commandPool->GetVkCommandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(Vk::GPU::gpu, &allocInfo, commandBuffers.data()), "Failed to allocate command buffers.");

	for (size_t i = 0; i < commandBuffers.size(); i++) 
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
		{
			THROW("Failed to begin recording command buffer.");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = Vk::DefaultPipeline::pipeline->GetRenderPass()->GetVkRenderPass();
		renderPassInfo.framebuffer = Vk::SwapChainRenderTarget::renderTarget->GetFramebuffers()[i]->GetVkFramebuffer();
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = Vk::SwapChain::extent;

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Vk::DefaultPipeline::pipeline->GetVkPipeline());
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(commandBuffers[i]), "Failed to record command buffer.");
	}
}

void CreateSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(Vk::SwapChain::images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
		VK_CHECK(vkCreateSemaphore(Vk::GPU::gpu, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]), "Failed to create semaphore.");
		VK_CHECK(vkCreateSemaphore(Vk::GPU::gpu, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]), "Failed to create semaphore.");
		VK_CHECK(vkCreateFence(Vk::GPU::gpu, &fenceInfo, nullptr, &inFlightFences[i]), "Failed to create fence.");
	}
}

void DestroySemaphores()
{	
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
	{
        vkDestroySemaphore(Vk::GPU::gpu, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(Vk::GPU::gpu, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(Vk::GPU::gpu, inFlightFences[i], nullptr);
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
static void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(g_PhysicalDevice, g_QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(g_PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
    VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
    wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(g_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    //printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(g_MinImageCount >= 2);;
    ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, wd, g_QueueFamily, g_Allocator, width, height, g_MinImageCount);
}

void Window::OnInit()
{
	Vk::Instance::Create();
	Vk::Surface::Create();
	Vk::GPU::Create();
	Vk::SwapChain::Create();

	Vk::DefaultPipeline::Create();
	Vk::SwapChainRenderTarget::Create(Vk::DefaultPipeline::pipeline->GetRenderPass()->GetVkRenderPass());
	Vk::DefaultCommandPool::Create();

	CreateCommandBuffers();
	CreateSyncObjects();

	g_Instance = Vk::Instance::instance;
	g_PhysicalDevice = Vk::GPU::physicalDevice;
	g_Device = Vk::GPU::gpu;
	g_QueueFamily = Vk::Queues::indices.graphicsFamily.value();
	g_Queue = Vk::Queues::graphicsQueue;

    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        VK_CHECK(vkCreateDescriptorPool(g_Device, &pool_info, g_Allocator, &g_DescriptorPool), "Failed to create descriptor pool.");
    }

    int w, h;
    glfwGetFramebufferSize(Window::glfwWindow, &w, &h);
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	wd->Width = w;
	wd->Height = h;
	wd->Swapchain = Vk::SwapChain::swapChain;
	wd->Surface = Vk::Surface::surface;
	wd->SurfaceFormat = Vk::SwapChain::surfaceFormat;
	wd->PresentMode = Vk::SwapChain::presentMode;
	wd->RenderPass = Vk::DefaultPipeline::pipeline->GetRenderPass()->GetVkRenderPass();
	wd->Pipeline = Vk::DefaultPipeline::pipeline->GetVkPipeline();
	wd->ClearEnable = true;
	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	wd->ClearValue = clearColor;
	wd->FrameIndex = 0;
	wd->ImageCount = g_MinImageCount;
	wd->SemaphoreIndex = 0;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
	
    ImGui_ImplGlfw_InitForVulkan(Window::glfwWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = g_Instance;
    init_info.PhysicalDevice = g_PhysicalDevice;
    init_info.Device = g_Device;
    init_info.QueueFamily = g_QueueFamily;
    init_info.Queue = g_Queue;
    init_info.PipelineCache = g_PipelineCache;
    init_info.DescriptorPool = g_DescriptorPool;
    init_info.Allocator = g_Allocator;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = wd->ImageCount;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

	
    {
        // Use any command queue
        VkCommandPool command_pool = Vk::DefaultCommandPool::commandPool->GetVkCommandPool();
        VkCommandBuffer command_buffer = commandBuffers[0];

        VK_CHECK(vkResetCommandPool(g_Device, command_pool, 0), "Fail");
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info), "Fail to begin");

        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &command_buffer;
        VK_CHECK(vkEndCommandBuffer(command_buffer), "Failed to end");
        VK_CHECK(vkQueueSubmit(g_Queue, 1, &end_info, VK_NULL_HANDLE), "Failed to submit");
        VK_CHECK(vkDeviceWaitIdle(g_Device), "Cant wait");
		
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }
}

static void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    VkResult err;

    // VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore image_acquired_semaphore  = imageAvailableSemaphores[wd->SemaphoreIndex];
    VkSemaphore render_complete_semaphore = renderFinishedSemaphores[wd->SemaphoreIndex];
    err = vkAcquireNextImageKHR(g_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        // g_SwapChainRebuild = true;
        return;
    }
    // check_vk_result(err);

    // ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        // err = vkWaitForFences(g_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        err = vkWaitForFences(g_Device, 1, &inFlightFences[wd->FrameIndex], VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking

        err = vkResetFences(g_Device, 1, &inFlightFences[wd->FrameIndex]);
    }
    {
        // err = vkResetCommandPool(g_Device, fd->CommandPool, 0);
        err = vkResetCommandPool(g_Device, Vk::DefaultCommandPool::commandPool->GetVkCommandPool(), 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(commandBuffers[wd->FrameIndex], &info);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = Vk::SwapChainRenderTarget::renderTarget->GetFramebuffers()[wd->FrameIndex]->GetVkFramebuffer();
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(commandBuffers[wd->FrameIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[wd->FrameIndex]);

    // Submit command buffer
    vkCmdEndRenderPass(commandBuffers[wd->FrameIndex]);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &commandBuffers[wd->FrameIndex];
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(commandBuffers[wd->FrameIndex]);
        err = vkQueueSubmit(g_Queue, 1, &info, inFlightFences[wd->FrameIndex]);
    }
}

static void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    // VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkSemaphore render_complete_semaphore = renderFinishedSemaphores[wd->SemaphoreIndex];

    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(g_Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
    {
        return;
    }

    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
}

void DrawFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	
	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	// wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
	// wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
	// wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
	// wd->ClearValue.color.float32[3] = clear_color.w;
    ImGui_ImplVulkanH_Window* wd = &g_MainWindowData;
	FrameRender(wd, draw_data);
	FramePresent(wd);

    // vkWaitForFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    // vkResetFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame]);

    // uint32_t imageIndex;
    // vkAcquireNextImageKHR(Vk::GPU::gpu, Vk::SwapChain::swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    // if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
	// {
    //     vkWaitForFences(Vk::GPU::gpu, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    // }
    // // Mark the image as now being in use by this frame
    // imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	// VkSubmitInfo submitInfo{};
	// submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	// VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	// submitInfo.waitSemaphoreCount = 1;
	// submitInfo.pWaitSemaphores = waitSemaphores;
	// submitInfo.pWaitDstStageMask = waitStages;
	// submitInfo.commandBufferCount = 1;
	// submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	// VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	// submitInfo.signalSemaphoreCount = 1;
	// submitInfo.pSignalSemaphores = signalSemaphores;

    // vkResetFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame]);
	// VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer.");

	// VkPresentInfoKHR presentInfo{};
	// presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// presentInfo.waitSemaphoreCount = 1;
	// presentInfo.pWaitSemaphores = signalSemaphores;

	// VkSwapchainKHR swapChains[] = { Vk::SwapChain::swapChain };
	// presentInfo.swapchainCount = 1;
	// presentInfo.pSwapchains = swapChains;
	// presentInfo.pImageIndices = &imageIndex;
	// presentInfo.pResults = nullptr; // Optional
	// vkQueuePresentKHR(Vk::Queues::presentQueue, &presentInfo);

    // currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Window::OnUpdate()
{
	glfwPollEvents();

	DrawFrame();
}

void Window::OnShutdown()
{
    vkDeviceWaitIdle(Vk::GPU::gpu);

	vkDestroyDescriptorPool(Vk::GPU::gpu, g_DescriptorPool, nullptr);

	DestroySemaphores();

	// Vk::SwapChainCommandBuffers::Destroy();
	Vk::DefaultCommandPool::Destroy();
	Vk::SwapChainRenderTarget::Destroy();
	Vk::DefaultPipeline::Destroy();

	Vk::SwapChain::Destroy();
	Vk::GPU::Destroy();
	Vk::Surface::Destroy();
	Vk::Instance::Destroy();
}

int main(int amountOfArguments, char *arguments[])
{
	Assets::LocateRoot(amountOfArguments, arguments);

	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}