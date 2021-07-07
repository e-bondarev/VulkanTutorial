cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(VulkanTutorial VERSION 0.0.1)

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE src src/*.cpp)

add_executable(VulkanTutorial ${src})

target_precompile_headers(VulkanTutorial PUBLIC src/pch.h)

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

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin/Release)
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug)
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/bin)