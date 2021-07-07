#include "window/window.h"

#include "vk/instance/instance.h"
#include "vk/gpu/gpu.h"
#include "vk/surface/surface.h"

void Window::OnInit()
{
	Vk::Instance::Create();
	Vk::Surface::Create();
	Vk::GPU::Create();
}

void Window::OnUpdate()
{
	glfwPollEvents();

	glfwSwapBuffers(Window::glfwWindow);
}

void Window::OnShutdown()
{
	Vk::GPU::Destroy();
	Vk::Surface::Destroy();
	Vk::Instance::Destroy();
}

int main()
{
	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}