#include "default_pipeline.h"

#include "../../assets/text_asset.h"

namespace Vk {
namespace DefaultPipeline {

Pipeline* pipeline;

void Create()
{
	Assets::Text vertexShader("assets/shaders/default.vert.spv");
	Assets::Text fragmentShader("assets/shaders/default.frag.spv");

	pipeline = new Pipeline(vertexShader.GetContent(), fragmentShader.GetContent());
}

void Destroy()
{
	delete pipeline;
}

}
}