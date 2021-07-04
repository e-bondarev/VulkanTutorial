cmake_minimum_required(VERSION 3.16)

project(VulkanTutorial VERSION 0.0.1)

file(GLOB_RECURSE src src/*.cpp)

add_executable(VulkanTutorial ${src})

target_precompile_headers(VulkanTutorial PUBLIC src/pch.h)

target_link_libraries(VulkanTutorial ${Vulkan_LIBRARIES})
target_include_directories(VulkanTutorial PUBLIC ${Vulkan_INCLUDE_DIR})