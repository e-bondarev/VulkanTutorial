#pragma once

#include "../../common.h"

#include "../../vk/frame/frame.h"
#include "../../vk/memory/buffer.h"
#include "../../vk/pipeline/pipeline.h"
#include "../../vk/commands/command_pool.h"
#include "../../vk/framebuffer/framebuffer.h"
#include "../../vk/commands/command_buffer.h"
#include "../../vk/descriptors/descriptor_set_layout.h"
#include "../../vk/descriptors/descriptor_pool.h"
#include "../../vk/descriptors/descriptor_set.h"

#include "../example.h"

#include "../common/ubo_struct.h"

namespace Examples
{
	class UniformBuffers : public IExample
	{
	public:
		UniformBuffers();
		void Render() override;
		~UniformBuffers() override;

		void BeforeResize() override;
		void AfterResize() override;

	private:
		Vk::Pipeline* pipeline;

		Vk::Buffer* vertexBuffer;
		Vk::Buffer* indexBuffer;

		struct
		{
			UBO data;
			std::vector<Vk::Buffer*> buffer;
			std::vector<Vk::DescriptorSetLayout*> setLayout;
		} ubo;

		Vk::DescriptorPool* descriptorPool;
		Vk::DescriptorSet* descriptorSet; 

		// 3 images.
		std::vector<Vk::Framebuffer*> framebuffers;
		std::vector<Vk::CommandPool*> commandPools;
		std::vector<Vk::CommandBuffer*> commandBuffers;
		std::vector<VkFence> imagesInFlight;

		// 2 frames.
		Vk::FrameManager* frameManager;
		
		// Render
		void RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer);
		void UpdateUBO();
		void Draw(Vk::CommandBuffer* command_buffer);
		void Present();

		UniformBuffers(const UniformBuffers&) = delete;
		UniformBuffers& operator=(const UniformBuffers&) = delete;
	};
}