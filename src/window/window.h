#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Window {

extern GLFWwindow* window;

void Create(int width = 800, int height = 600);
void Update();
void Shutdown();

void OnInit();
void OnUpdate();
void OnShutdown();

}