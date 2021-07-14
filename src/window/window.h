#pragma once

#include "../common.h"

namespace Window
{
	extern GLFWwindow *glfwWindow;
	extern glm::vec2 lastSize;
	extern glm::vec2 size;

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
	void OnResize();
	void OnShutdown();
}