#include "window/window.h"
#include "assets/assets.h"
#include "assets/text_asset.h"

#include "vk/instance/instance.h"

#include "vk/device/device.h"
#include "vk/device/queue_family.h"

#include "vk/surface/surface.h"
#include "vk/swap_chain/swap_chain.h"

#include "vk/pipeline/pipeline.h"
#include "vk/framebuffer/framebuffer.h"

Vk::Pipeline* pipeline;
std::vector<Vk::Framebuffer*> framebuffers;

void Window::OnInit()
{
	Vk::instance = new Vk::Instance();
	Vk::surface = new Vk::Surface();
	Vk::device = new Vk::Device();
	Vk::swapChain = new Vk::SwapChain();

	Assets::Text vs_code("assets/shaders/default.vert.spv");
	Assets::Text fs_code("assets/shaders/default.frag.spv");

	glm::vec2 viewport_size = { Vk::swapChain->GetExtent().width, Vk::swapChain->GetExtent().height };

	pipeline = new Vk::Pipeline(vs_code.GetContent(), fs_code.GetContent(), viewport_size, Vk::swapChain->GetImageFormat());

	// framebuffers.resize(Vk::swapChain->GetImageViews().size());

	for (const VkImageView& image_view : Vk::swapChain->GetImageViews())
	{
		framebuffers.push_back(new Vk::Framebuffer(image_view, pipeline->GetRenderPass()->GetVkRenderPass(), viewport_size));
	}
}

void Window::OnUpdate()
{
	glfwPollEvents();
}

void Window::OnShutdown()
{
	for (const Vk::Framebuffer* framebuffer : framebuffers)
	{
		delete framebuffer;
	}

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