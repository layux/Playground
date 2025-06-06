
add_executable(VulkanTriangle Sources/Entrypoint.cpp)

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

target_link_libraries(VulkanTriangle PRIVATE
    glfw
    glm::glm
    GPUOpen::VulkanMemoryAllocator
    spdlog::spdlog
    vk-bootstrap
    Vulkan::Vulkan)

if(WIN32)
    target_compile_definitions(VulkanTriangle PRIVATE "UNICODE" "_UNICODE" "PLATFORM_WINDOWS")
    #set_target_properties(VulkanTriangle PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    target_compile_definitions(VulkanTriangle PRIVATE "PLATFORM_DARWIN")
else()
    target_compile_definitions(VulkanTriangle PRIVATE "PLATFORM_LINUX")
endif()