#include "shaders.h"

#include "../gpu/gpu.h"

namespace Vk {
namespace Shaders {

VkShaderModule CreateModule(const std::string& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(GPU::gpu, &createInfo, nullptr, &shaderModule), "Failed to create shader module.");

	return shaderModule;
}

}
}