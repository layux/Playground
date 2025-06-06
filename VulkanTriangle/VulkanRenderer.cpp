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
        return false;
    }

    if (!CreateSyncObjects(renderer)) {
        return false;
    }

    return true;
}

void DestroyVulkanRenderer(VulkanRenderer& renderer) {
    DestroyCommandBuffers(renderer);
    DestroySyncObjects(renderer);
    DestroyVulkanSwapchain(renderer.swapchain, renderer.device);
    DestroyVulkanDevice(renderer.device);

    spdlog::info("Vulkan renderer destroyed successfully.");
}

bool CreateSyncObjects(VulkanRenderer& renderer) {
    VkDevice device = renderer.device.logicalDevice;
    
    // Set up max frames in flight
    renderer.sync.maxFramesInFlight = 3;
    renderer.sync.currentFrame = 0;
    
    // Resize sync objects vectors based on max frames in flight
    renderer.sync.imageAvailableSemaphores.resize(renderer.sync.maxFramesInFlight);
    renderer.sync.renderFinishedSemaphores.resize(renderer.sync.maxFramesInFlight);
    renderer.sync.inFlightFences.resize(renderer.sync.maxFramesInFlight);
    
    // Create semaphores and fences for each frame
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't wait
    
    for (size_t i = 0; i < renderer.sync.maxFramesInFlight; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderer.sync.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderer.sync.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &renderer.sync.inFlightFences[i]) != VK_SUCCESS) {
            spdlog::error("Failed to create synchronization objects for frame {}", i);
            return false;
        }
    }
    
    // Create fences for images in flight (one per swapchain image)
    renderer.sync.imagesInFlight.resize(renderer.swapchain.swapchainImages.size(), VK_NULL_HANDLE);
    
    spdlog::info("Created Vulkan synchronization objects");
    return true;
}

void DestroySyncObjects(VulkanRenderer& renderer) {
    VkDevice device = renderer.device.logicalDevice;
    
    for (size_t i = 0; i < renderer.sync.maxFramesInFlight; i++) {
        vkDestroySemaphore(device, renderer.sync.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderer.sync.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, renderer.sync.inFlightFences[i], nullptr);
    }
    
    spdlog::info("Destroyed Vulkan synchronization objects");
}

bool CreateCommandBuffers(VulkanRenderer& renderer) {
    VkDevice device = renderer.device.logicalDevice;
    
    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = renderer.device.graphicsQueueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &renderer.commands.commandPool) != VK_SUCCESS) {
        spdlog::error("Failed to create command pool");
        return false;
    }
    
    // Create command buffers (one per max frames in flight)
    renderer.commands.commandBuffers.resize(renderer.sync.maxFramesInFlight);
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer.commands.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer.commands.commandBuffers.size());
    
    if (vkAllocateCommandBuffers(device, &allocInfo, renderer.commands.commandBuffers.data()) != VK_SUCCESS) {
        spdlog::error("Failed to allocate command buffers");
        return false;
    }
    
    spdlog::info("Created Vulkan command buffers");
    return true;
}

void DestroyCommandBuffers(VulkanRenderer& renderer) {
    VkDevice device = renderer.device.logicalDevice;
    
    if (renderer.commands.commandPool != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(
            device, 
            renderer.commands.commandPool,
            static_cast<uint32_t>(renderer.commands.commandBuffers.size()),
            renderer.commands.commandBuffers.data()
        );
        
        vkDestroyCommandPool(device, renderer.commands.commandPool, nullptr);
        renderer.commands.commandPool = VK_NULL_HANDLE;
        
        spdlog::info("Destroyed Vulkan command buffers and pool");
    }
}

bool BeginFrame(VulkanRenderer& renderer, uint32_t& imageIndex) {
    VkDevice device = renderer.device.logicalDevice;
    
    // Wait for the previous frame to finish
    vkWaitForFences(device, 1, &renderer.sync.inFlightFences[renderer.sync.currentFrame], VK_TRUE, UINT64_MAX);
    
    // Acquire the next image from the swap chain
    VkResult result = vkAcquireNextImageKHR(
        device,
        renderer.swapchain.swapchain,
        UINT64_MAX,
        renderer.sync.imageAvailableSemaphores[renderer.sync.currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );
    
    // Handle swapchain recreation or other errors
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Swapchain is out of date and needs to be recreated
        // Note: In a real application, you'd recreate the swapchain here
        spdlog::warn("Swapchain out of date, skipping frame");
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::error("Failed to acquire swap chain image");
        return false;
    }
    
    // Check if a previous frame is using this image
    if (renderer.sync.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &renderer.sync.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    
    // Mark the image as now being in use by this frame
    renderer.sync.imagesInFlight[imageIndex] = renderer.sync.inFlightFences[renderer.sync.currentFrame];
    
    // Reset the fence for the current frame
    vkResetFences(device, 1, &renderer.sync.inFlightFences[renderer.sync.currentFrame]);
    
    // Record command buffer
    VkCommandBuffer commandBuffer = renderer.commands.commandBuffers[renderer.sync.currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);
    
    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;
    
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        spdlog::error("Failed to begin recording command buffer");
        return false;
    }
    
    // Begin render pass
    // Note: In a real application, you would set up the render pass here
    
    // Mark that a frame is in progress
    renderer.sync.frameInProgress = true;
    
    return true;
}

bool EndFrame(VulkanRenderer& renderer, uint32_t imageIndex) {
    VkDevice device = renderer.device.logicalDevice;
    
    // End command buffer recording
    VkCommandBuffer commandBuffer = renderer.commands.commandBuffers[renderer.sync.currentFrame];
    
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        spdlog::error("Failed to record command buffer");
        renderer.sync.frameInProgress = false;
        return false;
    }
    
    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {renderer.sync.imageAvailableSemaphores[renderer.sync.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    VkSemaphore signalSemaphores[] = {renderer.sync.renderFinishedSemaphores[renderer.sync.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(renderer.device.graphicsQueue, 1, &submitInfo, renderer.sync.inFlightFences[renderer.sync.currentFrame]) != VK_SUCCESS) {
        spdlog::error("Failed to submit draw command buffer");
        renderer.sync.frameInProgress = false;
        return false;
    }
    
    // Present the frame
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapchains[] = {renderer.swapchain.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    
    VkResult result = vkQueuePresentKHR(renderer.device.presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Swapchain is out of date or suboptimal
        // Note: In a real application, you'd recreate the swapchain here
        spdlog::warn("Swapchain out of date or suboptimal after presentation");
    } else if (result != VK_SUCCESS) {
        spdlog::error("Failed to present swap chain image");
        renderer.sync.frameInProgress = false;
        return false;
    }
    
    // Update current frame
    renderer.sync.currentFrame = (renderer.sync.currentFrame + 1) % renderer.sync.maxFramesInFlight;
    renderer.sync.frameInProgress = false;
    
    return true;
}