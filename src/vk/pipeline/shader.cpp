#include "shader.h"

#include "shaders.h"
#include "../gpu/gpu.h"

namespace Vk {

Shader::Shader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode)
{
	vertShaderModule = Shaders::CreateModule(vertexShaderCode);
	fragShaderModule = Shaders::CreateModule(fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	shaderStages = 
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};
}

Shader::~Shader()
{
    vkDestroyShaderModule(GPU::gpu, fragShaderModule, nullptr);
    vkDestroyShaderModule(GPU::gpu, vertShaderModule, nullptr);	
}

}