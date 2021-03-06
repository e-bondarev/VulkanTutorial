#include "window/window.h"
#include "assets/assets.h"

#include "examples/triangle/triangle.h"
#include "examples/imgui/imgui.h"
#include "examples/vertex_buffers/vertex_buffers.h"
#include "examples/uniform_buffers/uniform_buffers.h"
#include "examples/dynamic_uniform_buffers/dynamic_uniform_buffers.h"

#include "vk/vk.h"

Examples::IExample* example;

void Window::OnInit()
{
	Vk::Init();

	// example = new Examples::Triangle();
	// example = new Examples::ImGUI();
	// example = new Examples::VertexBuffers();
	// example = new Examples::UniformBuffers();
	example = new Examples::DynamicUniformBuffers();
}

void Window::OnResize()
{
	Vk::Global::device->WaitIdle();

	example->BeforeResize();
		delete Vk::Global::swapChain;
		Vk::Global::swapChain = new Vk::Global::SwapChain();
	example->AfterResize();
}

void Window::OnUpdate()
{
	example->Render();
}

void Window::OnShutdown()
{
	delete example;	
	Vk::Shutdown();
}

int main(int amountOfArguments, char *arguments[])
{
	Assets::LocateRoot(amountOfArguments, arguments);

	Window::Create(800, 600, Window::Mode::Windowed);
	Window::Update();
	Window::Shutdown();

    return 0;
}