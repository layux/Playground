#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"
#include "Window.hpp"

#include <spdlog/spdlog.h>
#include <VkBootstrap.h>

#include <algorithm>
#include <vulkan/vulkan.h>

// Helper function to choose swap surface format
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

// Helper function to choose swap present mode
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

// Helper function to choose swap extent
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window.handle, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

bool CreateVulkanSwapchain(VulkanSwapchain& swapchain, const VulkanDevice& device, const Window& window) {
    // Create surface
    if (glfwCreateWindowSurface(device.instance, window.handle, nullptr, &swapchain.surface) != VK_SUCCESS) {
        spdlog::error("Failed to create window surface!");
        return false;
    }
    spdlog::info("Successfully created window surface.");

    vkb::SwapchainBuilder swapchainBuilder{device.physicalDevice, device.logicalDevice, swapchain.surface};

    // Query for swapchain support details
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, swapchain.surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, swapchain.surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    if (formatCount != 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, swapchain.surface, &formatCount, formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, swapchain.surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (presentModeCount != 0) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, swapchain.surface, &presentModeCount, presentModes.data());
    }

    if (formats.empty() || presentModes.empty()) {
        spdlog::error("Swapchain support inadequate: no formats or present modes available.");
        return false;
    }

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    VkExtent2D extent = chooseSwapExtent(capabilities, window);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    vkb::Result<vkb::Swapchain> vkbSwapchainResult = swapchainBuilder
        .set_desired_format(surfaceFormat)
        .set_desired_present_mode(presentMode)
        .set_desired_extent(extent.width, extent.height)
        .set_desired_min_image_count(imageCount)
        .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .set_pre_transform_flags(capabilities.currentTransform)
        .set_composite_alpha_flags(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
        .set_clipped(VK_TRUE)
        .build();

    if (!vkbSwapchainResult) {
        spdlog::error("Failed to build swapchain: {}", vkbSwapchainResult.error().message());
        return false;
    }
    vkb::Swapchain vkbSwapchain = vkbSwapchainResult.value();

    swapchain.swapchain = vkbSwapchain.swapchain;
    swapchain.swapchainImageFormat = static_cast<VkFormat>(vkbSwapchain.image_format);
    swapchain.swapchainExtent = vkbSwapchain.extent;

    // Retrieve swapchain images
    auto imagesResult = vkbSwapchain.get_images();
    if (!imagesResult) {
        spdlog::error("Failed to get swapchain images: {}", imagesResult.error().message());
        return false;
    }
    swapchain.swapchainImages = imagesResult.value();

    auto imageViewsResult = vkbSwapchain.get_image_views();
    if (!imageViewsResult) {
        spdlog::error("Failed to get swapchain image views: {}", imageViewsResult.error().message());
        return false;
    }
    swapchain.swapchainImageViews = imageViewsResult.value();

    spdlog::info("Vulkan swapchain created successfully.");

    // Depth resources are typically created elsewhere or as part of a render pass setup
    // For now, we ensure they are initialized to VK_NULL_HANDLE as per VulkanSwapchain.hpp

    return true;
}

void DestroyVulkanSwapchain(VulkanSwapchain& swapchain, const VulkanDevice& device) {
    // Destroy depth resources if they were created (example)
    if (swapchain.depthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(device.logicalDevice, swapchain.depthImageView, nullptr);
        swapchain.depthImageView = VK_NULL_HANDLE;
    }
    // Assuming VMA is used for depthImageAllocation, proper destruction would be needed here
    // For example: if (swapchain.depthImage != VK_NULL_HANDLE && swapchain.depthImageAllocation != VK_NULL_HANDLE) {
    //    vmaDestroyImage(device.allocator, swapchain.depthImage, swapchain.depthImageAllocation);
    //    swapchain.depthImage = VK_NULL_HANDLE;
    //    swapchain.depthImageAllocation = VK_NULL_HANDLE;
    // }

    for (auto imageView : swapchain.swapchainImageViews) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device.logicalDevice, imageView, nullptr);
        }
    }
    swapchain.swapchainImageViews.clear();

    if (swapchain.swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device.logicalDevice, swapchain.swapchain, nullptr);
        swapchain.swapchain = VK_NULL_HANDLE;
    }

    if (swapchain.surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(device.instance, swapchain.surface, nullptr);
        swapchain.surface = VK_NULL_HANDLE;
    }

    spdlog::info("Vulkan swapchain destroyed successfully.");
}