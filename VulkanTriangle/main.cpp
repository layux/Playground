#include "VulkanRenderer.hpp"
#include "Window.hpp"

#include <spdlog/spdlog.h>

int main()
{
    // Initialize logging
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting Vulkan Triangle application");
    
    // Create window
    Window window{ "Vulkan Triangle", 1280, 720 };
    if (!CreateWindow(window)) {
        spdlog::error("Failed to create window");
        return EXIT_FAILURE;
    }
    
    // Create renderer
    VulkanRenderer renderer;
    if (!CreateVulkanRenderer(renderer, window)) {
        spdlog::error("Failed to create Vulkan renderer");
        DestroyWindow(window);
        return EXIT_FAILURE;
    }
    
    // Main rendering loop
    while (!glfwWindowShouldClose(window.handle)) {
        glfwPollEvents();

        // TODO: Update application state here
        
        // Render frame
        uint32_t imageIndex;
        if (BeginFrame(renderer, imageIndex)) {
            // Here you would record render commands
            // We'll just submit an empty frame for now
            EndFrame(renderer, imageIndex);
        }
    }
    
    // Cleanup
    DestroyVulkanRenderer(renderer);
    DestroyWindow(window);

    spdlog::info("Vulkan Triangle application terminated successfully");
    return EXIT_SUCCESS;
}