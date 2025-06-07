#pragma once

#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace MiniEngine::Graphics
{
    class Window;

    class VulkanDevice
    {
    public:
        VulkanDevice(Window *window);
        ~VulkanDevice();

    private:
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        uint32_t graphicsQueueFamilyIndex = 0;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        uint32_t presentQueueFamilyIndex = 0;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VmaAllocator allocator = VK_NULL_HANDLE;
    };
}