#include "window/window.h"

#include "vk/instance/instance.h"
#include "vk/gpu/gpu.h"
#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

void Window::OnInit()
{
	Vk::Instance::Create();
	Vk::Surface::Create();
	Vk::GPU::Create();
	Vk::SwapChain::Create();
}

void Window::OnUpdate()
{
	glfwPollEvents();

	glfwSwapBuffers(Window::glfwWindow);
}

void Window::OnShutdown()
{
	Vk::SwapChain::Destroy();
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