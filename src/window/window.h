#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Window
{
	extern GLFWwindow *glfwWindow;

	enum class Mode
	{
		Borderless,
		Fullscreen,
		Windowed
	};

	void Create(int width = 1920, int height = 1080, Mode mode = Mode::Borderless);
	void Update();
	void Shutdown();

	void OnInit();
	void OnUpdate();
	void OnShutdown();
}