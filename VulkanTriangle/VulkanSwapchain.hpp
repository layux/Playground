#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

struct VulkanDevice;
struct Window;

struct VulkanSwapchain {
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent = {};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkFormat depthImageFormat = VK_FORMAT_UNDEFINED;
    VkImage depthImage = VK_NULL_HANDLE;
    VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> swapchainFramebuffers;
};

bool CreateVulkanSwapchain(VulkanSwapchain& swapchain, const VulkanDevice& device, const Window& window);
void DestroyVulkanSwapchain(VulkanSwapchain& swapchain, const VulkanDevice& device);