#include "window/window.h"
#include "vk/instance/instance.h"
#include "vk/gpu/gpu.h"

void Window::OnInit()
{
	Vk::Instance::Create();
	Vk::GPU::Create();
}

void Window::OnUpdate()
{
	glfwPollEvents();

	glfwSwapBuffers(Window::window);
}

void Window::OnShutdown()
{
	Vk::GPU::Destroy();
	Vk::Instance::Destroy();
}

int main()
{
	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}