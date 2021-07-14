#pragma once

#include <spdlog/spdlog.h>

#include <algorithm>
#include <optional>
#include <vector>
#include <string>
#include <memory>
#include <array>
#include <set>

#define VOID_ASSEMBLY			(void(0))

#ifdef NDEBUG
#	define RELEASE
#else
#	define DEBUG
#endif

#ifdef RELEASE
#	define DISABLE_LOG
#	define DISABLE_CHECKS
#endif

#ifndef DISABLE_LOG
#	define LOG_OUT(...) 		spdlog::info(__VA_ARGS__)
#	define ERR_OUT(...) 		spdlog::error(__VA_ARGS__)
#	define WARN_OUT(...) 		spdlog::warn(__VA_ARGS__)
#	define TRACE()				LOG_OUT("[Call]: " __FUNCTION__)
#else
#	define LOG_OUT(...) 		VOID_ASSEMBLY
#	define ERR_OUT(...) 		VOID_ASSEMBLY
#	define WARN_OUT(...) 		VOID_ASSEMBLY
#	define TRACE()				VOID_ASSEMBLY
#endif

#ifndef DISABLE_CHECKS
#	define THROW(...)					\
		ERR_OUT(__VA_ARGS__);			\
		throw std::runtime_error("")

#	define VK_CHECK(x, msg)				\
		if (x != VK_SUCCESS)			\
		{								\
			THROW(msg);					\
		}
#else
#	define THROW(...)			VOID_ASSEMBLY
#	define VK_CHECK(x, msg)		x
#endif