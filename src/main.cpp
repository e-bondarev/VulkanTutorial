#include "window/window.h"
#include "assets/assets.h"

#include "vk/instance/instance.h"
#include "vk/gpu/gpu.h"
#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"
#include "vk/pipeline/default_pipeline.h"

void Window::OnInit()
{
	Vk::Instance::Create();
	Vk::Surface::Create();
	Vk::GPU::Create();
	Vk::SwapChain::Create();

	Vk::DefaultPipeline::Create();
}

void Window::OnUpdate()
{
	glfwPollEvents();

	glfwSwapBuffers(Window::glfwWindow);
}

void Window::OnShutdown()
{
	Vk::DefaultPipeline::Destroy();

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