#include "window.h"

namespace Window {

GLFWwindow* window = nullptr;

void Create(int width, int height)
{
    glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan Tutorial", nullptr, nullptr);
	OnInit();
}

void Update()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		OnUpdate();
		glfwSwapBuffers(window);
	}
}

void Shutdown() 
{
	OnShutdown();
	glfwDestroyWindow(window);
	glfwTerminate();
}

}