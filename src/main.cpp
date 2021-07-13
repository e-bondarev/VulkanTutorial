#include "window/window.h"
#include "assets/assets.h"
#include "assets/text_asset.h"

#include "vk/instance/instance.h"

#include "vk/device/device.h"
#include "vk/device/queue_family.h"

#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

#include "vk/pipeline/pipeline.h"

Vk::Pipeline* pipeline;

void Window::OnInit()
{
	Vk::instance = new Vk::Instance();
	Vk::surface = new Vk::Surface();
	Vk::device = new Vk::Device();
	Vk::swapChain = new Vk::SwapChain();

	Assets::Text vs_code("assets/shaders/default.vert.spv");
	Assets::Text fs_code("assets/shaders/default.frag.spv");
	pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), { Vk::swapChain->GetExtent().width, Vk::swapChain->GetExtent().height }, Vk::swapChain->GetImageFormat());
}

void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::OnShutdown()
{
	delete pipeline;

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