#include "window/window.h"
#include "assets/assets.h"

#include "vk/instance/instance.h"

#include "vk/device/device.h"
#include "vk/device/queue_family.h"

#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

void Window::OnInit()
{
	Vk::instance = new Vk::Instance();
	Vk::surface = new Vk::Surface();
	Vk::device = new Vk::Device();
	Vk::swapChain = new Vk::SwapChain();
}

void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::OnShutdown()
{
	delete Vk::swapChain;
	delete Vk::device;
	delete Vk::surface;
	delete Vk::instance;
}

int main(int amountOfArguments, char *arguments[])
{
	Assets::LocateRoot(amountOfArguments, arguments);

	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}