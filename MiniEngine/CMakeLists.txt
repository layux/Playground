file(GLOB_RECURSE MINI_ENGINE_SOURCES "Source/*.cpp")
add_library(MiniEngine STATIC ${MINI_ENGINE_SOURCES})

target_include_directories(MiniEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
        $<INSTALL_INTERFACE:Include>
)

target_link_libraries(MiniEngine
    PUBLIC
        glfw
        glm::glm
        GPUOpen::VulkanMemoryAllocator
        spdlog::spdlog
        vk-bootstrap
        Vulkan::Vulkan
)