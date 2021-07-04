#include <GLFW/glfw3.h>

int main()
{
    glfwInit();

    GLFWwindow* window;
    window = glfwCreateWindow(800, 600, "Vulkan Tutorial", nullptr, nullptr);

    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();

        glfwSwapBuffers(window);
    }

    return 0;
}