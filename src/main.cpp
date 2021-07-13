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

Vk::Pipeline* pipeline;
std::vector<Vk::Framebuffer*> framebuffers;
std::vector<Vk::CommandPool*> commandPools;
std::vector<Vk::CommandBuffer*> commandBuffers;

// VkSemaphore imageAvailable;
// VkSemaphore renderFinished;

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

std::vector<VkSemaphore> imageAvailable;
std::vector<VkSemaphore> renderFinished;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;

int currentFrame = 0;

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

	
    imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinished.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);

	// Create semaphores
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VK_CHECK(vkCreateSemaphore(Vk::device->GetVkDevice(), &semaphore_info, nullptr, &imageAvailable[i]), "Failed to create semaphore 1");
		VK_CHECK(vkCreateSemaphore(Vk::device->GetVkDevice(), &semaphore_info, nullptr, &renderFinished[i]), "Failed to create semaphore 2");
		VK_CHECK(vkCreateFence(Vk::device->GetVkDevice(), &fence_info, nullptr, &inFlightFences[i]), "Failed to create fence 1");
	}
}

void DrawFrame()
{	
    vkWaitForFences(Vk::device->GetVkDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(Vk::device->GetVkDevice(), 1, &inFlightFences[currentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(Vk::device->GetVkDevice(), Vk::swapChain->GetVkSwapChain(), UINT64_MAX, imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
        vkWaitForFences(Vk::device->GetVkDevice(), 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

	VkSemaphore waitSemaphores[] = { imageAvailable[currentFrame] };
	VkSemaphore signalSemaphores[] = { renderFinished[currentFrame] };

	// Render
	{
		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = waitSemaphores;
		submit_info.pWaitDstStageMask = waitStages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &commandBuffers[imageIndex]->GetVkCommandBuffer();	

		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signalSemaphores;

    	vkResetFences(Vk::device->GetVkDevice(), 1, &inFlightFences[currentFrame]);
		VK_CHECK(vkQueueSubmit(Vk::Queues::graphicsQueue, 1, &submit_info, inFlightFences[currentFrame]), "Failed to submit draw command buffer.");
	}

	// Present
	{
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { Vk::swapChain->GetVkSwapChain() };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = swapChains;
		present_info.pImageIndices = &imageIndex;
		present_info.pResults = nullptr; // Optional

		VK_CHECK(vkQueuePresentKHR(Vk::Queues::presentQueue, &present_info), "Failed to present.");
	}

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

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(Vk::device->GetVkDevice(), imageAvailable[i], nullptr);
		vkDestroySemaphore(Vk::device->GetVkDevice(), renderFinished[i], nullptr);
        vkDestroyFence(Vk::device->GetVkDevice(), inFlightFences[i], nullptr);
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