#pragma once

#include "../../common.h"

#include "../../vk/pipeline/pipeline.h"
#include "../../vk/framebuffer/framebuffer.h"
#include "../../vk/commands/command_pool.h"
#include "../../vk/commands/command_buffer.h"
#include "../../vk/descriptors/descriptor_pool.h"
#include "../../vk/frame/frame.h"

#include "../example.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_glfw.h"
#include "../../imgui/imgui_impl_vulkan.h"

namespace Examples
{
	class ImGUI : public IExample
	{
	public:
		ImGUI();
		void Render() override;
		void BeforeResize() override;
		void AfterResize() override;
		~ImGUI() override;

	private:
		Vk::Pipeline* pipeline;
		Vk::DescriptorPool* descriptorPool;

		// 3 images.
		std::vector<Vk::Framebuffer*> framebuffers;
		std::vector<Vk::CommandPool*> commandPools;
		std::vector<Vk::CommandBuffer*> commandBuffers;
		std::vector<VkFence> imagesInFlight;

		// 2 frames.
		Vk::FrameManager* frameManager;

		// Init
		void InitImGui();
		void ShutdownImGui();

		// Render
		void RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer);
		void Draw(Vk::CommandBuffer* command_buffer);
		void Present();

		ImGUI(const ImGUI&) = delete;
		ImGUI& operator=(const ImGUI&) = delete;
	};
}