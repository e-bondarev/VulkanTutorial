#pragma once

#include "../common.h"

#include "../vk/pipeline/pipeline.h"
#include "../vk/framebuffer/framebuffer.h"
#include "../vk/commands/command_pool.h"
#include "../vk/commands/command_buffer.h"

#include "example.h"

namespace Examples
{
	class Triangle : public IExample
	{
	public:
		Triangle();

		void Render() override;

		~Triangle() override;

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

		int currentFrame = 0;

		Triangle(const Triangle&) = delete;
		Triangle& operator=(const Triangle&) = delete;
	};
}