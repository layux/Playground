#define VMA_IMPLEMENTATION
#include "VulkanDevice.hpp"

#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <vector>

bool CreateVulkanDevice(VulkanDevice& device, const Window& window)
{
    // Instance creation with validation layers
    vkb::InstanceBuilder instanceBuilder;
    auto instanceResult = instanceBuilder
        .set_app_name("Vulkan Triangle")
        .request_validation_layers()
        .use_default_debug_messenger()
        .build();

    if (!instanceResult) {
        spdlog::error("Failed to create Vulkan instance: {}", instanceResult.error().message());
        return false;
    }
    
    auto vkbInstance = instanceResult.value();
    device.instance = vkbInstance.instance;
    device.debugMessenger = vkbInstance.debug_messenger;

    // Surface creation
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(device.instance, window.handle, nullptr, &surface) != VK_SUCCESS) {
        spdlog::error("Failed to create window surface");
        return false;
    }

    // Physical device selection
    vkb::PhysicalDeviceSelector physicalDeviceSelector(vkbInstance);
    auto physicalDeviceResult = physicalDeviceSelector
        .set_surface(surface)
        .set_minimum_version(1, 1)
        .require_dedicated_transfer_queue()
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
        .select();

    if (!physicalDeviceResult) {
        spdlog::error("Failed to select physical device: {}", physicalDeviceResult.error().message());
        return false;
    }

    spdlog::info("Selected physical device: {}", physicalDeviceResult.value().name);

    // Device creation
    vkb::DeviceBuilder deviceBuilder(physicalDeviceResult.value());
    auto deviceResult = deviceBuilder.build();
    if (!deviceResult) {
        spdlog::error("Failed to create logical device: {}", deviceResult.error().message());
        return false;
    }

    // Store device related handles
    auto vkbDevice = deviceResult.value();
    device.logicalDevice = vkbDevice.device;
    device.physicalDevice = vkbDevice.physical_device;

    // Get graphics queue
    auto graphicsQueueResult = vkbDevice.get_queue(vkb::QueueType::graphics);
    if (!graphicsQueueResult) {
        spdlog::error("Failed to get graphics queue: {}", graphicsQueueResult.error().message());
        return false;
    }
    device.graphicsQueue = graphicsQueueResult.value();
    device.graphicsQueueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // Get present queue
    auto presentQueueResult = vkbDevice.get_queue(vkb::QueueType::present);
    if (!presentQueueResult) {
        spdlog::error("Failed to get present queue: {}", presentQueueResult.error().message());
        return false;
    }
    device.presentQueue = presentQueueResult.value();
    device.presentQueueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::present).value();

    // Initialize VMA allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device.physicalDevice;
    allocatorInfo.device = device.logicalDevice;
    allocatorInfo.instance = device.instance;

    if (vmaCreateAllocator(&allocatorInfo, &device.allocator) != VK_SUCCESS) {
        spdlog::error("Failed to create VMA allocator");
        return false;
    }

    return true;
}

bool DestroyVulkanDevice(VulkanDevice& device)
{
    if (device.allocator) {
        vmaDestroyAllocator(device.allocator);
        device.allocator = VK_NULL_HANDLE;
    }
    
    if (device.logicalDevice) {
        vkDestroyDevice(device.logicalDevice, nullptr);
        device.logicalDevice = VK_NULL_HANDLE;
    }
    
    if (device.debugMessenger) {
        vkb::destroy_debug_utils_messenger(device.instance, device.debugMessenger);
        device.debugMessenger = VK_NULL_HANDLE;
    }
    
    if (device.instance) {
        vkDestroyInstance(device.instance, nullptr);
        device.instance = VK_NULL_HANDLE;
    }
    
    return true;
}
