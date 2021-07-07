#pragma once

#include "../../common.h"

namespace Vk {

class Shader
{
friend class Pipeline;

public:
	Shader(const std::string& vertexShaderCode, const std::string& fragmentShaderCode);
	~Shader();

private:
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
};

}