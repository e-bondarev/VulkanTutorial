#include "default_command_pool.h"

namespace Vk {
namespace DefaultCommandPool {

CommandPool* commandPool;

void Create()
{
	commandPool = new CommandPool();
}

void Destroy()
{
	delete commandPool;
}

}
}