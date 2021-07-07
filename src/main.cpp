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

std::vector<VkCommandBuffer> commandBuffers;

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;

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
}

void DrawFrame()
{
    vkWaitForFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(Vk::GPU::gpu, Vk::SwapChain::swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
	{
        vkWaitForFences(Vk::GPU::gpu, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(Vk::GPU::gpu, 1, &inFlightFences[currentFrame]);
	VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]), "Failed to submit draw command buffer.");

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { Vk::SwapChain::swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	vkQueuePresentKHR(Vk::Queues::presentQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Window::OnUpdate()
{
	glfwPollEvents();

	DrawFrame();
}

void Window::OnShutdown()
{
    vkDeviceWaitIdle(Vk::GPU::gpu);

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