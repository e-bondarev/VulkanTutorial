#include "window/window.h"
#include "assets/assets.h"

#include "examples/triangle/triangle.h"
#include "examples/imgui/imgui.h"

#include "vk/vk.h"

Examples::IExample* example;

void Window::OnInit()
{
	Vk::Init();

	// example = new Examples::Triangle();
	example = new Examples::ImGUI();
}

void Window::OnUpdate()
{
	glfwPollEvents();
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

	Window::Create(1920, 1080, Window::Mode::Borderless);
	Window::Update();
	Window::Shutdown();

    return 0;
}