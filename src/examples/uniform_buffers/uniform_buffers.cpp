#include "uniform_buffers.h"

#include "../../assets/text_asset.h"
#include "../../window/window.h"

#include "../../vk/instance/instance.h"
#include "../../vk/surface/surface.h"
#include "../../vk/device/device.h"
#include "../../vk/device/queue_family.h"
#include "../../vk/swap_chain/swap_chain.h"

#include "../common/vertex.h"

namespace Examples
{
	UniformBuffers::UniformBuffers()
	{
		frameManager = new Vk::FrameManager();

		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/uniform_buffers/uniform_buffers.vert.spv");
		Assets::Text fs_code("assets/shaders/uniform_buffers/uniform_buffers.frag.spv");

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
		{
			ubo.setLayout.push_back(new Vk::DescriptorSetLayout());
			ubo.buffer.push_back(new Vk::Buffer(
				sizeof(UBO), 1, &ubo.data, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			));
		}
		
		pipeline = new Vk::Pipeline(
			vs_code.GetContent(), 
			fs_code.GetContent(), 
			viewport_size, 
			Vk::Global::swapChain->GetImageFormat(), 
			Vertex::GetBindingDescriptions(), Vertex::GetAttributeDescriptions(),
			{ ubo.setLayout[0]->GetVkDescriptorSetLayout() }
		);

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			commandPools.push_back(new Vk::CommandPool());

		for (Vk::CommandPool* command_pool : commandPools)
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));

		{			
			const std::vector<Vertex> vertices = 
			{
				{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
				{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
			};

			Vk::Buffer staging_buffer(sizeof(Vertex), static_cast<uint32_t>(vertices.size()), vertices.data());
			vertexBuffer = new Vk::Buffer(&staging_buffer);
		}
		{
			const std::vector<uint16_t> indices = 
			{
				0, 1, 2, 2, 3, 0
			};

			Vk::Buffer staging_buffer(sizeof(uint16_t), static_cast<uint32_t>(indices.size()), indices.data());
			indexBuffer = new Vk::Buffer(&staging_buffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		}

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);

		std::vector<VkDescriptorSetLayout> layouts;
		for (int i = 0; i < ubo.setLayout.size(); i++)
		{
			layouts.push_back(ubo.setLayout[i]->GetVkDescriptorSetLayout());
		}

		descriptorPool = new Vk::DescriptorPool();
		descriptorSet = new Vk::DescriptorSet(descriptorPool, layouts.data(), static_cast<uint32_t>(layouts.size()));

		for (int i = 0; i < descriptorSet->GetVkDescriptorSets().size(); i++)
		{
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = ubo.buffer[i]->GetVkBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UBO);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSet->GetVkDescriptorSets()[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(Vk::Global::device->GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void UniformBuffers::RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* command_buffer, Vk::Framebuffer* framebuffer)
	{
		command_pool->Reset();
			command_buffer->Begin();
				command_buffer->BeginRenderPass(pipeline->GetRenderPass(), framebuffer);
					command_buffer->BindPipeline(pipeline);

						command_buffer->BindVertexBuffers({ vertexBuffer }, { 0 });
						command_buffer->BindIndexBuffer(indexBuffer);

						vkCmdBindDescriptorSets(command_buffer->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVkPipelineLayout(), 0, 1, &descriptorSet->GetVkDescriptorSets()[frameManager->GetCurrentFrameIndex()], 0, nullptr);

							command_buffer->DrawIndexed(indexBuffer->GetAmountOfElements(), 1, 0, 0, 0);

				command_buffer->EndRenderPass();
			command_buffer->End();
	}

	void UniformBuffers::UpdateUBO()
	{
		static float alpha = 0.0f;

		alpha += 0.01f;

		ubo.data.Model = glm::mat4x4(1);
		ubo.data.Model = glm::rotate(ubo.data.Model, glm::radians(alpha), glm::vec3(0, 0, 1));
		ubo.data.Model = glm::translate(ubo.data.Model, glm::vec3(0, 0, -10));

		ubo.data.View = glm::mat4x4(1);
		ubo.data.Projection = glm::perspective(glm::radians(70.0f), Window::size.x / Window::size.y, 0.1f, 1000.0f);

		ubo.buffer[frameManager->GetCurrentFrameIndex()]->Update(&ubo.data);
	}

	void UniformBuffers::Draw(Vk::CommandBuffer* command_buffer)
	{
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		command_buffer->SubmitToQueue(
			Vk::Global::Queues::graphicsQueue, 
			&current_frame->GetImageAvailableSemaphore(), 
			&current_frame->GetRenderFinishedSemaphore(), 
			current_frame->GetInFlightFence()
		);
	}

	void UniformBuffers::Present()
	{
		VkResult result = Vk::Global::swapChain->Present(&frameManager->GetCurrentFrame()->GetRenderFinishedSemaphore(), 1);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		{
			
		} 
		else if (result != VK_SUCCESS) 
		{
			THROW("Can't present.");
		}

		frameManager->NextFrame();
	}

	void UniformBuffers::Render()
	{		
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		uint32_t image_index = Vk::Global::swapChain->AcquireImage(current_frame->GetImageAvailableSemaphore());

		if (imagesInFlight[image_index] != VK_NULL_HANDLE) 
			vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &imagesInFlight[image_index], VK_TRUE, UINT64_MAX);

		vkWaitForFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence(), VK_TRUE, UINT64_MAX);
		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		imagesInFlight[image_index] = current_frame->GetInFlightFence();

		Vk::CommandPool* current_command_pool = commandPools[image_index];
		Vk::CommandBuffer* current_command_buffer = commandBuffers[image_index];	
		Vk::Framebuffer* current_framebuffer = framebuffers[image_index];

		RecordCommandBuffer(current_command_pool, current_command_buffer, current_framebuffer);
		UpdateUBO();
		Draw(current_command_buffer);
		Present();
	}

	UniformBuffers::~UniformBuffers()
	{
		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());
		
		delete descriptorPool;
		delete descriptorSet;

		delete frameManager;

		delete indexBuffer;
		delete vertexBuffer;

		for (const Vk::CommandPool* command_pool : commandPools)
		{
			delete command_pool;
		}

		for (const Vk::Framebuffer* framebuffer : framebuffers)
		{
			delete framebuffer;
		}

		delete pipeline;

		for (const Vk::DescriptorSetLayout* layout : ubo.setLayout)
		{
			delete layout;
		}

		for (const Vk::Buffer* buffer : ubo.buffer)
		{
			delete buffer;
		}
	}

	void UniformBuffers::BeforeResize()
	{
		for (const Vk::Framebuffer* framebuffer : framebuffers)
			delete framebuffer;
			
		for (const Vk::CommandBuffer* command_buffer : commandBuffers)
			delete command_buffer;

		framebuffers.clear();
		commandBuffers.clear();

		imagesInFlight.clear();

		delete pipeline;
	}

	void UniformBuffers::AfterResize()
	{
		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/uniform_buffers/uniform_buffers.vert.spv");
		Assets::Text fs_code("assets/shaders/uniform_buffers/uniform_buffers.frag.spv");
		
		pipeline = new Vk::Pipeline(
			vs_code.GetContent(), 
			fs_code.GetContent(), 
			viewport_size, 
			Vk::Global::swapChain->GetImageFormat(), 
			Vertex::GetBindingDescriptions(), Vertex::GetAttributeDescriptions()
		);

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));

		for (Vk::CommandPool* command_pool : commandPools)
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);
	}
}