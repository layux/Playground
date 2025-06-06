#pragma once

#include "Window.hpp"
#include <vk_mem_alloc.h>

struct VulkanDevice
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex = 0;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t presentQueueFamilyIndex = 0;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
};

bool CreateVulkanDevice(VulkanDevice& device, const Window& window);
bool DestroyVulkanDevice(VulkanDevice& device);