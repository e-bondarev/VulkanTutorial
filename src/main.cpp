#include "window/window.h"
#include "assets/assets.h"

#include "vk/instance/instance.h"

#include "vk/gpu/gpu.h"
#include "vk/gpu/queue_family.h"

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
}

void Window::OnShutdown()
{
	Vk::SwapChain::Destroy();
	Vk::GPU::Destroy();
	Vk::Surface::Destroy();
	Vk::Instance::Destroy();
}

int main(int amountOfArguments, char *arguments[])
{
	Assets::LocateRoot(amountOfArguments, arguments);

	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}