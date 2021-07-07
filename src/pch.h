#pragma once

#include <spdlog/spdlog.h>

#include <vector>
#include <string>
#include <optional>

#define LOG_OUT(...) 	spdlog::info(__VA_ARGS__)
#define ERR_OUT(...) 	spdlog::error(__VA_ARGS__)
#define WARN_OUT(...) 	spdlog::warn(__VA_ARGS__)

#define THROW(...)					\
	ERR_OUT(__VA_ARGS__);			\
	throw std::runtime_error("")

#define VK_CHECK(x, msg)			\
	if (x != VK_SUCCESS)			\
	{								\
		THROW(msg);					\
	}