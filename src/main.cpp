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

struct {
	Vk::DescriptorPool* descriptorPool;
} imGuiState;

void InitImGui()
{
	imGuiState.descriptorPool = new Vk::DescriptorPool();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(Window::glfwWindow, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Vk::instance->GetVkInstance();
    init_info.PhysicalDevice = Vk::device->GetVkPhysicalDevice();
    init_info.Device = Vk::device->GetVkDevice();
    init_info.QueueFamily = Vk::Queues::indices.graphicsFamily.value();
    init_info.Queue = Vk::Queues::graphicsQueue;
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = imGuiState.descriptorPool->GetVkDescriptorPool();
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

	my_command_buffer.SubmitToQueue(Vk::Queues::graphicsQueue);

	vkDeviceWaitIdle(Vk::device->GetVkDevice());
	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ShutdownImGui()
{	
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	delete imGuiState.descriptorPool;
}

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

	// InitImGui();
}

void Render()
{
	vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);

	commandBuffers[Vk::swapChain->GetCurrentImageIndex()]->SubmitToQueue(
		Vk::Queues::graphicsQueue, 
		&frames[currentFrame].ImageAvailable, 
		&frames[currentFrame].RenderFinished, 
		frames[currentFrame].InFlightFence
	);
}

void Present()
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &frames[currentFrame].RenderFinished;

	VkSwapchainKHR swapChains[] = { Vk::swapChain->GetVkSwapChain() };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;
	uint32_t image_index = Vk::swapChain->GetCurrentImageIndex();
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr; // Optional

	VK_CHECK(vkQueuePresentKHR(Vk::Queues::presentQueue, &present_info), "Failed to present.");

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void DrawImGuiFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
		ImGui::ShowDemoWindow();
	ImGui::Render();

	Vk::swapChain->AcquireImage(frames[currentFrame].ImageAvailable);

    if (imagesInFlight[Vk::swapChain->GetCurrentImageIndex()] != VK_NULL_HANDLE) 
        vkWaitForFences(Vk::device->GetVkDevice(), 1, &imagesInFlight[Vk::swapChain->GetCurrentImageIndex()], VK_TRUE, UINT64_MAX);

    vkWaitForFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);

    imagesInFlight[Vk::swapChain->GetCurrentImageIndex()] = frames[currentFrame].InFlightFence;

	Vk::CommandPool* current_command_pool = commandPools[currentFrame];
	Vk::CommandBuffer* current_command_buffer = commandBuffers[currentFrame];	
	Vk::Framebuffer* current_framebuffer = framebuffers[Vk::swapChain->GetCurrentImageIndex()];

	current_command_pool->Reset();
	current_command_buffer->Begin();
		pipeline->GetRenderPass()->Begin(current_command_buffer, current_framebuffer);
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), current_command_buffer->GetVkCommandBuffer());
		pipeline->GetRenderPass()->End(current_command_buffer);
	current_command_buffer->End();

	vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);
	
	current_command_buffer->SubmitToQueue(
		Vk::Queues::graphicsQueue, 
		&frames[currentFrame].ImageAvailable, 
		&frames[currentFrame].RenderFinished, 
		frames[currentFrame].InFlightFence
	);

	Present();
}

void RecordCommandBuffer()
{
	Vk::CommandPool* current_command_pool = commandPools[currentFrame];
	Vk::CommandBuffer* current_command_buffer = commandBuffers[currentFrame];	
	Vk::Framebuffer* current_framebuffer = framebuffers[Vk::swapChain->GetCurrentImageIndex()];

	current_command_pool->Reset();
	current_command_buffer->Begin();
		pipeline->GetRenderPass()->Begin(current_command_buffer, current_framebuffer);
			vkCmdBindPipeline(current_command_buffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipeline());
			vkCmdDraw(current_command_buffer->GetVkCommandBuffer(), 3, 1, 0, 0);
		pipeline->GetRenderPass()->End(current_command_buffer);
	current_command_buffer->End();
}

void DrawFrame()
{
	Vk::swapChain->AcquireImage(frames[currentFrame].ImageAvailable);

    if (imagesInFlight[Vk::swapChain->GetCurrentImageIndex()] != VK_NULL_HANDLE) 
        vkWaitForFences(Vk::device->GetVkDevice(), 1, &imagesInFlight[Vk::swapChain->GetCurrentImageIndex()], VK_TRUE, UINT64_MAX);

    vkWaitForFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Vk::device->GetVkDevice(), 1, &frames[currentFrame].InFlightFence);

    imagesInFlight[Vk::swapChain->GetCurrentImageIndex()] = frames[currentFrame].InFlightFence;
	
	RecordCommandBuffer();
	Render();
	Present();
}

void Window::OnUpdate()
{
	glfwPollEvents();

	DrawFrame();
	// DrawImGuiFrame();
}

void Window::OnShutdown()
{
	vkDeviceWaitIdle(Vk::device->GetVkDevice());

	// ShutdownImGui();

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