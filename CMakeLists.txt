# Minimum CMake version required
cmake_minimum_required(VERSION 3.28)

# Set the project name
project(Playground)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global dependencies
find_package(Vulkan REQUIRED)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.15.3)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG        v1.4.313)
FetchContent_MakeAvailable(vk-bootstrap)

FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.3.0)
FetchContent_MakeAvailable(VulkanMemoryAllocator)

# Include sub-directories
add_subdirectory(MiniEngine)
add_subdirectory(VulkanTriangle)