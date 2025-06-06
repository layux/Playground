#pragma once

#include "VulkanDevice.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSwapchain.hpp"

#include <vulkan/vulkan.h>

#include <vector>

struct VulkanSynchronization
{
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;
    size_t                   currentFrame      = 0;
    size_t                   maxFramesInFlight = 3;
    bool                     frameInProgress   = false;
};

struct VulkanCommands
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers; // one per frame in-flight
};

struct VulkanRenderer
{
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanSynchronization sync;
    VulkanCommands commands;
};

struct Mesh
{
    VkBuffer        vertexBuffer   = VK_NULL_HANDLE;
    VmaAllocation   vertexAlloc    = nullptr;
    uint32_t        vertexCount    = 0;

    VkBuffer        indexBuffer    = VK_NULL_HANDLE;
    VmaAllocation   indexAlloc     = nullptr;
    uint32_t        indexCount     = 0; // 0 means: draw vertices only
};

struct Renderable
{
    Mesh*           mesh      = nullptr;
    VulkanPipeline* pipeline  = nullptr;
};

// Create and destroy the renderer
bool CreateVulkanRenderer(VulkanRenderer& context, const Window& window);
void DestroyVulkanRenderer(VulkanRenderer& context);

// Frame management functions
bool BeginFrame(VulkanRenderer& context, uint32_t& imageIndex);
bool EndFrame(VulkanRenderer& context, uint32_t imageIndex);
