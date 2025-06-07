#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <string>
#include <vector>

/// Data structures
struct Window
{
	std::string title;
	uint32_t width;
	uint32_t height;
	GLFWwindow* handle = nullptr;
};

// Define these structs before they are used
struct VulkanDevice
{
	VkInstance               instance                 = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger           = VK_NULL_HANDLE;
	VkSurfaceKHR             surface                  = VK_NULL_HANDLE;
	VkPhysicalDevice         physicalDevice           = VK_NULL_HANDLE;
	VkDevice                 logicalDevice            = VK_NULL_HANDLE;
	uint32_t                 graphicsQueueFamilyIndex = 0;
	VkQueue                  graphicsQueue            = VK_NULL_HANDLE;
	uint32_t                 presentQueueFamilyIndex  = 0;
	VkQueue                  presentQueue             = VK_NULL_HANDLE;
	VmaAllocator             allocator                = VK_NULL_HANDLE;
};

struct VulkanSwapChain
{
	// Main presentation resources
	VkSwapchainKHR             handle               = VK_NULL_HANDLE;
	VkFormat                   imageFormat          = VK_FORMAT_UNDEFINED;
	VkExtent2D                 extent               = {};
	std::vector<VkImage>       images;
	std::vector<VkImageView>   imageViews;
	// Depth resources
	VkFormat                   depthFormat          = VK_FORMAT_UNDEFINED;
	VkImage                    depthImage           = VK_NULL_HANDLE;
	VmaAllocation              depthImageAllocation = VK_NULL_HANDLE;
	VkImageView                depthImageView       = VK_NULL_HANDLE;
	// Framebuffers (one per image in the swap chain)
	std::vector<VkFramebuffer> framebuffers;
};

struct VulkanSynchronization
{
	// Semaphores for image acquisition and presentation
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	// Fences for frame synchronization
	std::vector<VkFence>     inFlightFences;
	std::vector<VkFence>     imagesInFlight;
	uint32_t                 currentFrame      = 0;     // Index of the current frame
	uint32_t                 maxFramesInFlight = 2;     // Double buffering
	bool                     frameStarted      = false; // Indicates if a frame is currently being processed
};

struct VulkanRenderer
{
	VulkanDevice                 device;         // Use composition instead of pointers
	VulkanSwapChain              swapChain;      // Use composition instead of pointers
	VulkanSynchronization        synchronization;// Use composition instead of pointers
	VkCommandPool                commandPool     = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
};

/// Function declarations
bool initWindow(Window& window);
bool initVulkanRenderer(VulkanRenderer& renderer, const Window& window);
bool createVulkanDevice(VulkanDevice& device, const Window& window);
bool createSwapChain(VulkanSwapChain& swapChain, VulkanDevice& device, const Window& window);
bool createSynchronization(VulkanSynchronization& sync, VulkanDevice& device, const VulkanSwapChain& swapChain); // Added

void destroySynchronization(VulkanSynchronization& sync, VulkanDevice& device); // Added
void destroySwapChain(VulkanSwapChain& swapChain, VulkanDevice& device);
void destroyVulkanDevice(VulkanDevice& device);
void destroyVulkanRenderer(VulkanRenderer& renderer);
void destroyWindow(Window& window);

int main(int argc, char** argv)
{
	spdlog::set_level(spdlog::level::debug); // Set log level to debug
	spdlog::info("Vulkan Triangle Application Starting...");

	Window window = { "Vulkan Triangle", 800, 600 };
	if (!initWindow(window))
	{
		spdlog::critical("Failed to initialize GLFW window");
		destroyWindow(window);
		return EXIT_FAILURE;
	}

	VulkanRenderer renderer;
	if (!initVulkanRenderer(renderer, window))
	{
		spdlog::critical("Failed to initialize Vulkan renderer");
		destroyWindow(window);
		return EXIT_FAILURE;
	}

	spdlog::info("Application initialization complete");

	while (!glfwWindowShouldClose(window.handle))
	{
		glfwPollEvents();
		// Render frame (not implemented in this snippet)
		// ...
	}

	spdlog::info("Attempting to terminate gracefully");

	destroyVulkanRenderer(renderer);
	destroyWindow(window);
	spdlog::info("Application terminated");

	return EXIT_SUCCESS;
}


// -----------------------------------------------------------------------------
// Window management functions
// -----------------------------------------------------------------------------
bool initWindow(Window& window)
{
	if (!glfwInit())
	{
		spdlog::critical("Failed to initialize GLFW");
		return false;
	}

	spdlog::debug("GLFW initialized successfully");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);   // Allow resizing

	window.handle = glfwCreateWindow(window.width, window.height, window.title.c_str(), nullptr, nullptr);

	if (!window.handle)
	{
		spdlog::critical("Failed to create GLFW window: {}", window.title);
		glfwTerminate();
		return false;
	}

	spdlog::info("GLFW window created: {}", window.title);
	return true;
}

void destroyWindow(Window& window)
{
	if (window.handle)
	{
		spdlog::debug("Window handle is not null, destroying window: {}", window.title);
		glfwDestroyWindow(window.handle);
		window.handle = nullptr;
	}

	spdlog::debug("Terminating GLFW");
	glfwTerminate();

	spdlog::info("Window destroyed: {}", window.title);
}


// -----------------------------------------------------------------------------
// Vulkan 
// -----------------------------------------------------------------------------
bool initVulkanRenderer(VulkanRenderer& renderer, const Window& window)
{
	// Initialize the device directly within the renderer
	if (!createVulkanDevice(renderer.device, window))
	{
		spdlog::error("Failed to create Vulkan device");
		return false;
	}
	spdlog::info("Vulkan device created successfully");

	// Create SwapChain
	if (!createSwapChain(renderer.swapChain, renderer.device, window))
	{
		spdlog::error("Failed to create Vulkan swap chain");
		// No need to explicitly call destroyVulkanDevice here, 
		// as the caller of initVulkanRenderer will handle it if this function returns false.
		return false;
	}
	spdlog::info("Vulkan swap chain created successfully");

	// Create Synchronization objects
	if (!createSynchronization(renderer.synchronization, renderer.device, renderer.swapChain))
	{
		spdlog::error("Failed to create Vulkan synchronization objects");
		destroySwapChain(renderer.swapChain, renderer.device); // Clean up created swapchain
		return false;
	}
	spdlog::info("Vulkan synchronization objects created successfully");
	
	return true;
}

bool createVulkanDevice(VulkanDevice& device, const Window& window)
{
	// Create Vulkan instance using VkBootstrap
	vkb::InstanceBuilder instanceBuilder;
	auto instanceResult = instanceBuilder.set_app_name(window.title.c_str())
		.set_engine_name("MiniEngine")
		.request_validation_layers(true)
		.use_default_debug_messenger()
		.require_api_version(1, 2, 0)
		.build();

	if (!instanceResult)
	{
		spdlog::critical("Failed to create Vulkan instance: {}", instanceResult.error().message());
		return false;
	}

	vkb::Instance vkbInstance = instanceResult.value();
	device.instance = vkbInstance.instance;
	device.debugMessenger = vkbInstance.debug_messenger;

	spdlog::debug("Vulkan instance created successfully");

	// Create surface using GLFW
	if (glfwCreateWindowSurface(device.instance, window.handle, nullptr, &device.surface) != VK_SUCCESS)
	{
		spdlog::critical("Failed to create Vulkan surface");
		return false;
	}

	spdlog::debug("Vulkan surface created successfully");

	// Select physical device
	vkb::PhysicalDeviceSelector deviceSelector{ vkbInstance, device.surface };
	auto physicalDeviceResult = deviceSelector.set_minimum_version(1, 2)
		.select();

	if (!physicalDeviceResult)
	{
		spdlog::critical("Failed to select physical device: {}", physicalDeviceResult.error().message());
		return false;
	}

	vkb::PhysicalDevice vkbPhysicalDevice = physicalDeviceResult.value();
	device.physicalDevice = vkbPhysicalDevice.physical_device;
	spdlog::info("Physical device selected: {}", vkbPhysicalDevice.name);

	// Create logical device
	vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
	auto logicalDeviceResult = deviceBuilder.build();

	if (!logicalDeviceResult)
	{
		spdlog::critical("Failed to create logical device: {}", logicalDeviceResult.error().message());
		return false;
	}

	vkb::Device vkbDevice = logicalDeviceResult.value();
	device.logicalDevice = vkbDevice.device;

	spdlog::debug("Logical device created successfully");

	// Get graphics queue family and present queue
	device.graphicsQueueFamilyIndex = static_cast<uint32_t>(vkbDevice.get_queue_index(vkb::QueueType::graphics).value());
	device.graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	device.presentQueueFamilyIndex = static_cast<uint32_t>(vkbDevice.get_queue_index(vkb::QueueType::present).value());
	device.presentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();

	spdlog::debug("Graphics queue family index: {}, Present queue family index: {}",
		device.graphicsQueueFamilyIndex, device.presentQueueFamilyIndex);

	// Create VMA allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2; // Use Vulkan 1.2 API
	allocatorInfo.physicalDevice = device.physicalDevice;
	allocatorInfo.device = device.logicalDevice;
	allocatorInfo.instance = device.instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT; // Enable buffer device address

	if (vmaCreateAllocator(&allocatorInfo, &device.allocator) != VK_SUCCESS)
	{
		spdlog::critical("Failed to create VMA allocator");
		return false;
	}

	return true;
}

bool createSwapChain(VulkanSwapChain& swapChain, VulkanDevice& device, const Window& window)
{
    // Corrected SwapchainBuilder instantiation
    vkb::SwapchainBuilder swapchainBuilder(device.physicalDevice, 
                                           device.logicalDevice, 
                                           device.surface, 
                                           device.graphicsQueueFamilyIndex, 
                                           device.presentQueueFamilyIndex);

    auto swapchainResult = swapchainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) // FIFO is a good default and widely supported
        .set_desired_extent(window.width, window.height)
        //.set_desired_min_image_count(3) // Optional: request triple buffering
        .build();

    if (!swapchainResult)
    {
        spdlog::critical("Failed to create swap chain: {}", swapchainResult.error().message());
        return false;
    }

    auto vkbSwapchain = swapchainResult.value();
    swapChain.handle = vkbSwapchain.swapchain; // Corrected member name

    auto imagesResult = vkbSwapchain.get_images();
    if (!imagesResult) {
        spdlog::critical("Failed to get swap chain images: {}", imagesResult.error().message());
        // vkb::destroy_swapchain is not directly available, cleanup is handled by vkDestroySwapchainKHR
        // in destroySwapChain if this function returns false and initVulkanRenderer handles device cleanup.
        // If vkbSwapchain.swapchain was successfully created, it will be cleaned up by destroySwapChain.
        return false;
    }
    swapChain.images = imagesResult.value();
    swapChain.imageFormat = vkbSwapchain.image_format;
    swapChain.extent = vkbSwapchain.extent;

    // Create image views
    swapChain.imageViews.resize(swapChain.images.size());
    for (size_t i = 0; i < swapChain.images.size(); ++i)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapChain.images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapChain.imageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.logicalDevice, &viewInfo, nullptr, &swapChain.imageViews[i]) != VK_SUCCESS)
        {
            spdlog::critical("Failed to create image views for swap chain");
            // Cleanup already created image views before returning
            for (size_t j = 0; j < i; ++j) {
                vkDestroyImageView(device.logicalDevice, swapChain.imageViews[j], nullptr);
            }
            swapChain.imageViews.clear();
            // The swapchain itself (vkbSwapchain.swapchain) will be cleaned up by destroySwapChain
            // if this function returns false and initVulkanRenderer handles device cleanup.
            return false;
        }
    }
    spdlog::info("Swap chain created successfully with {} images.", swapChain.images.size());
    return true;
}

bool createSynchronization(VulkanSynchronization& sync, VulkanDevice& device, const VulkanSwapChain& swapChain)
{
    sync.imageAvailableSemaphores.assign(sync.maxFramesInFlight, VK_NULL_HANDLE);
    sync.renderFinishedSemaphores.assign(sync.maxFramesInFlight, VK_NULL_HANDLE);
    sync.inFlightFences.assign(sync.maxFramesInFlight, VK_NULL_HANDLE);
    
    // imagesInFlight should be sized based on the number of swap chain images
    // It stores VK_NULL_HANDLE or a pointer to one of the inFlightFences
    sync.imagesInFlight.assign(swapChain.images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Create fences in signaled state for the first frame

    bool success = true;
    for (uint32_t i = 0; i < sync.maxFramesInFlight; ++i)
    {
        if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &sync.imageAvailableSemaphores[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create imageAvailableSemaphore #{}", i);
            success = false; break;
        }
        if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &sync.renderFinishedSemaphores[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create renderFinishedSemaphore #{}", i);
            success = false; break;
        }
        if (vkCreateFence(device.logicalDevice, &fenceInfo, nullptr, &sync.inFlightFences[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create inFlightFence #{}", i);
            success = false; break;
        }
    }

    if (!success) {
        spdlog::critical("Failed to create all synchronization objects. Cleaning up partially created ones.");
        // Cleanup all potentially created sync objects by this call
        // This relies on destroySynchronization being able to handle partially filled/VK_NULL_HANDLE vectors
        // For a more direct cleanup here:
        for (uint32_t i = 0; i < sync.maxFramesInFlight; ++i) { // Iterate up to maxFramesInFlight as assign was used
            if (sync.imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.logicalDevice, sync.imageAvailableSemaphores[i], nullptr);
            }
            if (sync.renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.logicalDevice, sync.renderFinishedSemaphores[i], nullptr);
            }
            if (sync.inFlightFences[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device.logicalDevice, sync.inFlightFences[i], nullptr);
            }
        }
        sync.imageAvailableSemaphores.clear();
        sync.renderFinishedSemaphores.clear();
        sync.inFlightFences.clear();
        sync.imagesInFlight.clear();
        return false;
    }

    spdlog::info("Synchronization primitives created successfully ({} frames in flight).", sync.maxFramesInFlight);
    return true;
}

void destroyVulkanRenderer(VulkanRenderer& renderer)
{
	spdlog::debug("Destroying Vulkan renderer resources");
    // Destroy in reverse order of creation
    destroySynchronization(renderer.synchronization, renderer.device);
	destroySwapChain(renderer.swapChain, renderer.device);
	destroyVulkanDevice(renderer.device);
    spdlog::info("Vulkan renderer resources destroyed.");
}

void destroyVulkanDevice(VulkanDevice& device)
{
	if (device.allocator != VK_NULL_HANDLE)
	{
		spdlog::debug("Destroying VMA allocator");
		vmaDestroyAllocator(device.allocator);
		device.allocator = VK_NULL_HANDLE;
	}

	if (device.logicalDevice != VK_NULL_HANDLE)
	{
		spdlog::debug("Destroying logical device");
		vkDestroyDevice(device.logicalDevice, nullptr);
		device.logicalDevice = VK_NULL_HANDLE;
	}

	if (device.surface != VK_NULL_HANDLE)
	{
		spdlog::debug("Destroying Vulkan surface");
		vkDestroySurfaceKHR(device.instance, device.surface, nullptr);
		device.surface = VK_NULL_HANDLE;
	}

	if (device.debugMessenger != VK_NULL_HANDLE)
	{
		spdlog::debug("Destroying debug messenger");
		vkb::destroy_debug_utils_messenger(device.instance, device.debugMessenger, nullptr);
		device.debugMessenger = VK_NULL_HANDLE;
	}

	if (device.instance != VK_NULL_HANDLE)
	{
		spdlog::debug("Destroying Vulkan instance");
		vkDestroyInstance(device.instance, nullptr);
		device.instance = VK_NULL_HANDLE;
	}
}

void destroySwapChain(VulkanSwapChain& swapChain, VulkanDevice& device)
{
	for (auto imageView : swapChain.imageViews)
	{
		if (imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(device.logicalDevice, imageView, nullptr);
		}
	}
	swapChain.imageViews.clear();

	if (swapChain.handle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device.logicalDevice, swapChain.handle, nullptr);
		swapChain.handle = VK_NULL_HANDLE;
	}
	
	// Clear other swapchain members if necessary, e.g., images, format, extent
	swapChain.images.clear();
	swapChain.imageFormat = VK_FORMAT_UNDEFINED;
	swapChain.extent = {};

	spdlog::debug("Swap chain destroyed");
}

void destroySynchronization(VulkanSynchronization& sync, VulkanDevice& device)
{
    spdlog::debug("Destroying synchronization primitives...");
    for (VkSemaphore semaphore : sync.imageAvailableSemaphores) {
        if (semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
        }
    }
    sync.imageAvailableSemaphores.clear();

    for (VkSemaphore semaphore : sync.renderFinishedSemaphores) {
        if (semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(device.logicalDevice, semaphore, nullptr);
        }
    }
    sync.renderFinishedSemaphores.clear();

    for (VkFence fence : sync.inFlightFences) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device.logicalDevice, fence, nullptr);
        }
    }
    sync.inFlightFences.clear();
    
    sync.imagesInFlight.clear(); // Does not own Vulkan objects, just clears the vector

    spdlog::debug("Synchronization primitives destroyed.");
}