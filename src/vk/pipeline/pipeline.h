#pragma once

#include "shader.h"
#include "render_pass.h"

namespace Vk {

class Pipeline
{
public:
	Pipeline(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
	~Pipeline();

	const std::shared_ptr<RenderPass>& GetRenderPass() const;

private:
	VkPipeline pipeline;

	std::shared_ptr<Shader> shader;

	VkPipelineLayout layout;
	void SetupPipeline();
	void DestroyPipelineLayout();

	std::shared_ptr<RenderPass> renderPass;

	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
};

}