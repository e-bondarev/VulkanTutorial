#pragma once

#include "../common.h"

#include "../vk/pipeline/pipeline.h"
#include "../vk/framebuffer/framebuffer.h"
#include "../vk/commands/command_pool.h"
#include "../vk/commands/command_buffer.h"

#include "../vk/descriptors/descriptor_pool.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_vulkan.h"

#include "example.h"

namespace Examples
{
	class ImGUI : public IExample
	{
	public:
		ImGUI();

		void Render() override;

		~ImGUI() override;

	private:
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

		Vk::DescriptorPool* descriptorPool;

		int currentFrame = 0;

		void InitImGui();
		void ShutdownImGui();

		ImGUI(const ImGUI&) = delete;
		ImGUI& operator=(const ImGUI&) = delete;
	};
}