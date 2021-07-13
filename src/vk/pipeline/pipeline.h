#pragma once

#include "../../common.h"

#include "shader.h"
#include "render_pass.h"

namespace Vk
{
	class Pipeline
	{
	public:
		Pipeline(const std::string& vs_code, const std::string& fs_code, const glm::vec2& viewport_size, VkFormat image_format);
		~Pipeline();

		VkPipelineLayout GetVkPipelineLayout() const;
		VkPipeline GetVkPipeline() const;
		const RenderPass* GetRenderPass() const;

	private:
		Shader* shader;
		RenderPass* renderPass;

		VkPipelineLayout vkPipelineLayout;
		VkPipeline vkPipeline;

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
	};
}