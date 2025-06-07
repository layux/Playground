
add_executable(VulkanTriangle Sources/Entrypoint.cpp)

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