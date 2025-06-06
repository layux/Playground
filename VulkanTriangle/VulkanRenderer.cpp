#include "VulkanRenderer.hpp"

#include "Window.hpp"

#include <spdlog/spdlog.h>

bool CreateSyncObjects(VulkanRenderer& renderer);
bool CreateCommandBuffers(VulkanRenderer& renderer);
void DestroyCommandBuffers(VulkanRenderer& renderer);
void DestroySyncObjects(VulkanRenderer& renderer);

bool CreateVulkanRenderer(VulkanRenderer& renderer, const Window& window) {
    if (!CreateVulkanDevice(renderer.device, window)) {
        return false;
    }

    if (!CreateVulkanSwapchain(renderer.swapchain, renderer.device, window)) {
        DestroyVulkanDevice(renderer.device);
        return false;
    }

    CreateSyncObjects(renderer);
    CreateCommandBuffers(renderer);

    spdlog::info("Vulkan renderer created successfully with swapchain format: {} and extent: {}x{}",
                 renderer.swapchain.swapchainImageFormat,
                 renderer.swapchain.swapchainExtent.width,
                 renderer.swapchain.swapchainExtent.height);

    return true;
}

void DestroyVulkanRenderer(VulkanRenderer& renderer) {
    DestroyCommandBuffers(renderer);
    DestroySyncObjects(renderer);
    DestroyVulkanSwapchain(renderer.swapchain, renderer.device);
    DestroyVulkanDevice(renderer.device);

    spdlog::info("Vulkan renderer destroyed successfully.");
}