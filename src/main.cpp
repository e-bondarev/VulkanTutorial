#include "window/window.h"
#include "assets/assets.h"

#include "examples/triangle.h"
#include "examples/imgui.h"

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

	Window::Create();
	Window::Update();
	Window::Shutdown();

    return 0;
}