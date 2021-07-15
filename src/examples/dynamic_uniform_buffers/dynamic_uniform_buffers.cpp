#include "dynamic_uniform_buffers.h"

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
	DynamicUniformBuffers::DynamicUniformBuffers()
	{
		frameManager = new Vk::FrameManager();

		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/uniform_buffers/uniform_buffers.vert.spv");
		Assets::Text fs_code("assets/shaders/uniform_buffers/uniform_buffers.frag.spv");

		ubo.setLayout = new Vk::DescriptorSetLayout();

		for (int i = 0; i < frameManager->GetAmountOfFrames(); i++)
		{
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
			{ ubo.setLayout->GetVkDescriptorSetLayout() }
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

		descriptorPool = new Vk::DescriptorPool();
		
		for (int i = 0; i < frameManager->GetAmountOfFrames(); i++)
			descriptorSets.push_back(new Vk::DescriptorSet(descriptorPool, &ubo.setLayout->GetVkDescriptorSetLayout()));

		for (int i = 0; i < descriptorSets.size(); i++)
			descriptorSets[i]->Update(ubo.buffer[i], sizeof(UBO));
	}

	void DynamicUniformBuffers::RecordCommandBuffer(Vk::CommandPool* command_pool, Vk::CommandBuffer* cmd, Vk::Framebuffer* framebuffer)
	{
		Vk::DescriptorSet* current_descriptor_set = descriptorSets[frameManager->GetCurrentFrameIndex()];

		command_pool->Reset();

		cmd->Begin();
			cmd->BeginRenderPass(pipeline->GetRenderPass(), framebuffer);
				cmd->BindPipeline(pipeline);
					cmd->BindVertexBuffers({ vertexBuffer }, { 0 });
					cmd->BindIndexBuffer(indexBuffer);
						cmd->BindDescriptorSets(pipeline, { &current_descriptor_set->GetVkDescriptorSet() }, 1);
							cmd->DrawIndexed(indexBuffer->GetAmountOfElements(), 1, 0, 0, 0);
			cmd->EndRenderPass();
		cmd->End();
	}

	void DynamicUniformBuffers::UpdateUBO()
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

	void DynamicUniformBuffers::Draw(Vk::CommandBuffer* cmd)
	{
		Vk::Frame* current_frame = frameManager->GetCurrentFrame();

		vkResetFences(Vk::Global::device->GetVkDevice(), 1, &current_frame->GetInFlightFence());

		cmd->SubmitToQueue(
			Vk::Global::Queues::graphicsQueue, 
			&current_frame->GetImageAvailableSemaphore(), 
			&current_frame->GetRenderFinishedSemaphore(), 
			current_frame->GetInFlightFence()
		);
	}

	void DynamicUniformBuffers::Present()
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

	void DynamicUniformBuffers::Render()
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

	DynamicUniformBuffers::~DynamicUniformBuffers()
	{
		vkDeviceWaitIdle(Vk::Global::device->GetVkDevice());
		
		delete descriptorPool;

		for (const auto& descriptor_set : descriptorSets)
			delete descriptor_set;

		delete frameManager;

		delete indexBuffer;
		delete vertexBuffer;

		for (const Vk::CommandPool* command_pool : commandPools)
			delete command_pool;

		for (const Vk::Framebuffer* framebuffer : framebuffers)
			delete framebuffer;

		delete pipeline;

		delete ubo.setLayout;

		for (const Vk::Buffer* buffer : ubo.buffer)
			delete buffer;
	}

	void DynamicUniformBuffers::BeforeResize()
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

	void DynamicUniformBuffers::AfterResize()
	{
		glm::vec2 viewport_size = { Vk::Global::swapChain->GetExtent().width, Vk::Global::swapChain->GetExtent().height };

		Assets::Text vs_code("assets/shaders/uniform_buffers/uniform_buffers.vert.spv");
		Assets::Text fs_code("assets/shaders/uniform_buffers/uniform_buffers.frag.spv");
		
		pipeline = new Vk::Pipeline(
			vs_code.GetContent(), 
			fs_code.GetContent(), 
			viewport_size, 
			Vk::Global::swapChain->GetImageFormat(), 
			Vertex::GetBindingDescriptions(), Vertex::GetAttributeDescriptions(),
			{ ubo.setLayout->GetVkDescriptorSetLayout() }
		);

		for (const VkImageView& image_view : Vk::Global::swapChain->GetImageViews())
			framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));

		for (Vk::CommandPool* command_pool : commandPools)
			commandBuffers.push_back(new Vk::CommandBuffer(command_pool));

		imagesInFlight.resize(framebuffers.size(), VK_NULL_HANDLE);
	}
}