cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
project(VulkanTutorial VERSION 0.0.1)

file(GLOB_RECURSE src src/*.cpp)

add_executable(VulkanTutorial ${src})

target_precompile_headers(VulkanTutorial PUBLIC src/pch.h)

find_package(Vulkan REQUIRED)

add_subdirectory(external/glfw)
add_subdirectory(external/spdlog)

set(LINK_LIBRARIES 
    glfw
	spdlog
    ${Vulkan_LIBRARIES}
)

set(INCLUDE_DIRS 
    external/glfw/include
    external/glm
    external/spdlog/include
    ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(VulkanTutorial ${LINK_LIBRARIES})
target_include_directories(VulkanTutorial PUBLIC ${INCLUDE_DIRS})