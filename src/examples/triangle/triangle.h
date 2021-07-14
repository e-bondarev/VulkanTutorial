#pragma once

#include "../../common.h"

#include "../../vk/pipeline/pipeline.h"
#include "../../vk/framebuffer/framebuffer.h"
#include "../../vk/commands/command_pool.h"
#include "../../vk/commands/command_buffer.h"

#include "../../vk/frame/frame.h"

#include "../example.h"

namespace Examples
{
	class Triangle : public IExample
	{
	public:
		Triangle();
		void Render() override;
		~Triangle() override;

	private:
		void RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer);
		void Draw(Vk::CommandBuffer* command_buffer);
		void Present();

		Vk::Pipeline* pipeline;

		// 3 images.
		std::vector<Vk::Framebuffer*> framebuffers;
		std::vector<Vk::CommandPool*> commandPools;
		std::vector<Vk::CommandBuffer*> commandBuffers;
		std::vector<VkFence> imagesInFlight;

		// 2 frames.
		Vk::FrameManager* frameManager;

		Triangle(const Triangle&) = delete;
		Triangle& operator=(const Triangle&) = delete;
	};
}