#pragma once

#include "../../common.h"

#include <memory>

namespace Vk 
{
	class Instance 
	{
	public:
		Instance();
		~Instance();

		VkInstance GetVkInstance() const;

	private:
		VkInstance vkInstance;

		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
	};
	
	extern Instance* instance;
}