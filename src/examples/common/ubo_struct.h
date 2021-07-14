#pragma once

#include "../../common.h"

namespace Examples
{
	struct UBO
	{
		glm::mat4x4 Model;
		glm::mat4x4 View;
		glm::mat4x4 Projection;
	};
}