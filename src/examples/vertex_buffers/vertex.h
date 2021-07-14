#pragma once

#include "../../common.h"

namespace Examples
{
	struct Vertex
	{
		glm::vec2 Position;
		glm::vec3 Color;

		static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
	};
}