#include "window.h"

namespace Window {

GLFWwindow* glfwWindow = nullptr;

void Create(int width, int height)
{
    glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindow = glfwCreateWindow(width, height, "Vulkan Tutorial", nullptr, nullptr);
	OnInit();
}

void Update()
{
	while (!glfwWindowShouldClose(glfwWindow))
	{
		glfwPollEvents();
		OnUpdate();
		glfwSwapBuffers(glfwWindow);
	}
}

void Shutdown() 
{
	OnShutdown();
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

}