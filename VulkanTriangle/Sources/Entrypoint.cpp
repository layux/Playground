#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <string>
#include <vector>
#include <fstream> // For readFile
#include <glm/glm.hpp> // For Vertex struct

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
	// Semaphores for image acquisition and presentation - one pair per swap chain image
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	// Fences for frame synchronization - one per frame in flight
	std::vector<VkFence>     inFlightFences;
	// Track which fence is associated with each swap chain image
	// This prevents rendering to images that are still in flight
	std::vector<VkFence>     imagesInFlight;
	uint32_t                 currentFrame      = 0;     // Index of the current frame
	uint32_t                 maxFramesInFlight = 2;     // Double buffering
	bool                     frameStarted      = false; // Indicates if a frame is currently being processed
};

// --- New Data Structures ---
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    // glm::vec2 texCoord; // Could be added later for texturing
};

struct VulkanMesh {
    VkBuffer      vertexBuffer       = VK_NULL_HANDLE;
    VmaAllocation vertexBufferMemory = VK_NULL_HANDLE;
    uint32_t      vertexCount        = 0;
    // VkBuffer      indexBuffer        = VK_NULL_HANDLE; // Optional for indexed drawing
    // VmaAllocation indexBufferMemory  = VK_NULL_HANDLE;
    // uint32_t      indexCount         = 0;
};

struct VulkanPipeline {
    VkPipelineLayout pipelineLayout   = VK_NULL_HANDLE;
    VkRenderPass     renderPass       = VK_NULL_HANDLE;
    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;
    // VkShaderModule   vertShaderModule = VK_NULL_HANDLE; // Optional: if managed by pipeline
    // VkShaderModule   fragShaderModule = VK_NULL_HANDLE; // Optional: if managed by pipeline
};
// --- End New Data Structures ---

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

// Utility Functions
std::vector<char> readFile(const std::string& filename);
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
VkVertexInputBindingDescription getVertexBindingDescription();
std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

// Mesh Lifecycle
bool createVertexBuffer(VulkanMesh& mesh, VulkanDevice& device, VmaAllocator allocator, const std::vector<Vertex>& vertices);
void destroyMesh(VulkanMesh& mesh, VulkanDevice& device, VmaAllocator allocator);

// Pipeline Lifecycle
bool createRenderPass(VkRenderPass& renderPass, VulkanDevice& device, VkFormat swapChainImageFormat);
void destroyRenderPass(VkRenderPass& renderPass, VulkanDevice& device); // If render pass is managed separately

bool createGraphicsPipeline(
    VulkanPipeline& pipeline,
    VulkanDevice& device,
    VkExtent2D swapChainExtent,
    VkRenderPass compatibleRenderPass,
    const std::string& vertShaderPath,
    const std::string& fragShaderPath
);
void destroyVulkanPipeline(VulkanPipeline& pipeline, VulkanDevice& device);

// Framebuffer Lifecycle
bool createFramebuffers(VulkanSwapChain& swapChain, VulkanDevice& device, VkRenderPass renderPass);
void destroyFramebuffers(VulkanSwapChain& swapChain, VulkanDevice& device);

// Command Pool & Buffer Management
bool createCommandPool(VulkanRenderer& renderer);
bool createCommandBuffers(VulkanRenderer& renderer); // Renamed from createPrimaryCommandBuffers for clarity

// Drawing Operations
bool drawFrame(
    VulkanRenderer& renderer,
    VulkanPipeline& activePipeline,
    VulkanMesh& meshToDraw
);

void recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    VulkanRenderer& renderer,
    VulkanPipeline& activePipeline,
    VulkanMesh& meshToDraw
);
// --- End New Function Declarations ---

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

	// Create a basic render pass
	VkRenderPass renderPass = VK_NULL_HANDLE;
	if (!createRenderPass(renderPass, renderer.device, renderer.swapChain.imageFormat))
	{
		spdlog::critical("Failed to create render pass");
		destroyVulkanRenderer(renderer);
		destroyWindow(window);
		return EXIT_FAILURE;
	}
	spdlog::info("Render pass created successfully");

	// Create framebuffers for the swap chain
	if (!createFramebuffers(renderer.swapChain, renderer.device, renderPass))
	{
		spdlog::critical("Failed to create framebuffers");
		// Clean up created resources
		destroyRenderPass(renderPass, renderer.device);
		destroyVulkanRenderer(renderer);
		destroyWindow(window);
		return EXIT_FAILURE;
	}
	spdlog::info("Framebuffers created successfully");
	// Create a graphics pipeline for our triangle
	VulkanPipeline pipeline;
	pipeline.renderPass = renderPass;
	
	// Compile shaders if needed
	// Note: In production, this would be part of your build process
	system("compile_shaders.bat");
	
	// Create the graphics pipeline with our vertex and fragment shaders
	if (!createGraphicsPipeline(
	    pipeline,
	    renderer.device,
	    renderer.swapChain.extent,
	    renderPass,
	    "Resources/Shaders/spirv/Triangle.vert.spv",
	    "Resources/Shaders/spirv/Triangle.frag.spv"
	)) {
	    spdlog::critical("Failed to create graphics pipeline");
	    destroyFramebuffers(renderer.swapChain, renderer.device);
	    destroyRenderPass(renderPass, renderer.device);
	    destroyVulkanRenderer(renderer);
	    destroyWindow(window);
	    return EXIT_FAILURE;
	}
	spdlog::info("Graphics pipeline created successfully");

	// Create a triangle mesh
	// For this shader, we'll use a hardcoded triangle in the shader itself
	// (see Triangle.vert), so we just need a dummy mesh with 3 vertices
	VulkanMesh triangleMesh;
	std::vector<Vertex> vertices(3);
	
	// The actual vertex data will be defined in the shader
	// We just need to create a buffer with the correct number of vertices
	if (!createVertexBuffer(triangleMesh, renderer.device, renderer.device.allocator, vertices)) {
	    spdlog::critical("Failed to create vertex buffer");
	    destroyVulkanPipeline(pipeline, renderer.device);
	    destroyFramebuffers(renderer.swapChain, renderer.device);
	    destroyRenderPass(renderPass, renderer.device);
	    destroyVulkanRenderer(renderer);
	    destroyWindow(window);
	    return EXIT_FAILURE;
	}
	spdlog::info("Triangle mesh created successfully with {} vertices", triangleMesh.vertexCount);

	spdlog::info("Application initialization complete");

	while (!glfwWindowShouldClose(window.handle))
	{
		glfwPollEvents();
				// Draw a frame with our triangle
		if (!drawFrame(renderer, pipeline, triangleMesh))
		{
			// Handle swap chain recreation or other errors
			spdlog::warn("Failed to draw frame");
		}
	}
	// Wait for the device to finish all operations before cleanup
	vkDeviceWaitIdle(renderer.device.logicalDevice);
	
	spdlog::info("Attempting to terminate gracefully");
	// Clean up resources in reverse order of creation
	destroyMesh(triangleMesh, renderer.device, renderer.device.allocator);
	destroyVulkanPipeline(pipeline, renderer.device);
	destroyFramebuffers(renderer.swapChain, renderer.device);
	destroyRenderPass(renderPass, renderer.device);
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
	
	// Create Command Pool
	if (!createCommandPool(renderer))
	{
		spdlog::error("Failed to create Vulkan command pool");
		destroySynchronization(renderer.synchronization, renderer.device);
		destroySwapChain(renderer.swapChain, renderer.device);
		return false;
	}
	spdlog::info("Command pool created successfully");

	// Create Command Buffers
	if (!createCommandBuffers(renderer))
	{
		spdlog::error("Failed to create Vulkan command buffers");
		// Command pool will be destroyed by destroyVulkanRenderer
		destroySynchronization(renderer.synchronization, renderer.device);
		destroySwapChain(renderer.swapChain, renderer.device);
		return false;
	}
	spdlog::info("Command buffers allocated successfully");
	
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
	// Remove VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT flag since bufferDeviceAddress feature is not enabled

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
    // Create semaphores per swap chain image (not per frame in flight)
    uint32_t imageCount = static_cast<uint32_t>(swapChain.images.size());
    sync.imageAvailableSemaphores.assign(imageCount, VK_NULL_HANDLE);
    sync.renderFinishedSemaphores.assign(imageCount, VK_NULL_HANDLE);
    
    // We still maintain one fence per frame in flight for CPU-GPU synchronization
    sync.inFlightFences.assign(sync.maxFramesInFlight, VK_NULL_HANDLE);
    
    // imagesInFlight still sized based on the number of swap chain images
    // It stores VK_NULL_HANDLE or a pointer to one of the inFlightFences
    sync.imagesInFlight.assign(imageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Create fences in signaled state for the first frame

    bool success = true;
    
    // Create semaphores for each swap chain image
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &sync.imageAvailableSemaphores[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create imageAvailableSemaphore #{}", i);
            success = false; break;
        }
        if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &sync.renderFinishedSemaphores[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create renderFinishedSemaphore #{}", i);
            success = false; break;
        }
    }
    
    // Create fences for each frame in flight
    for (uint32_t i = 0; i < sync.maxFramesInFlight && success; ++i)
    {
        if (vkCreateFence(device.logicalDevice, &fenceInfo, nullptr, &sync.inFlightFences[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create inFlightFence #{}", i);
            success = false; break;
        }
    }    if (!success) {
        spdlog::critical("Failed to create all synchronization objects. Cleaning up partially created ones.");
        // Cleanup all potentially created sync objects by this call
        
        // Clean up semaphores (per swap chain image)
        for (uint32_t i = 0; i < sync.imageAvailableSemaphores.size(); ++i) {
            if (sync.imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.logicalDevice, sync.imageAvailableSemaphores[i], nullptr);
            }
            if (i < sync.renderFinishedSemaphores.size() && sync.renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(device.logicalDevice, sync.renderFinishedSemaphores[i], nullptr);
            }
        }
        
        // Clean up fences (per frame in flight)
        for (uint32_t i = 0; i < sync.inFlightFences.size(); ++i) {
            if (sync.inFlightFences[i] != VK_NULL_HANDLE) {
                vkDestroyFence(device.logicalDevice, sync.inFlightFences[i], nullptr);
            }
        }
        
        // Clear all vectors
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
    
    // Destroy command pool (this implicitly frees all allocated command buffers)
    if (renderer.commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(renderer.device.logicalDevice, renderer.commandPool, nullptr);
        renderer.commandPool = VK_NULL_HANDLE;
        spdlog::debug("Command pool destroyed");
    }
    
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
    // Destroy image-specific semaphores
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

    // Destroy frame-specific fences
    for (VkFence fence : sync.inFlightFences) {
        if (fence != VK_NULL_HANDLE) {
            vkDestroyFence(device.logicalDevice, fence, nullptr);
        }
    }
    sync.inFlightFences.clear();
    
    sync.imagesInFlight.clear(); // Does not own Vulkan objects, just clears the vector

    spdlog::debug("Synchronization primitives destroyed.");
}

// --- New Function Definitions ---
// Utility Functions
std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        spdlog::critical("Failed to open file: {}", filename);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    spdlog::debug("Read {} bytes from file: {}", fileSize, filename);
    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        spdlog::critical("Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    spdlog::debug("Shader module created successfully");
    return shaderModule;
}

// Vertex Input Descriptions (Vulkan-specific for our Vertex struct)
VkVertexInputBindingDescription getVertexBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions() {
    return {
        // Position attribute
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) },
        // Color attribute
        { 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
        // TexCoord attribute (if added in the future)
        // { 0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) },
    };
}

// Mesh Lifecycle
bool createVertexBuffer(VulkanMesh& mesh, VulkanDevice& device, VmaAllocator allocator, const std::vector<Vertex>& vertices) {
    mesh.vertexCount = static_cast<uint32_t>(vertices.size());    // Create the vertex buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(Vertex) * mesh.vertexCount;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assuming single queue family for now

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // Host visible memory for mapping
    // Don't set the requiredFlags to allow VMA to choose appropriate memory type

    if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &mesh.vertexBuffer, &mesh.vertexBufferMemory, nullptr) != VK_SUCCESS) {
        spdlog::critical("Failed to create vertex buffer");
        return false;
    }

    // Copy vertex data to the buffer
    void* data;
    if (vmaMapMemory(allocator, mesh.vertexBufferMemory, &data) != VK_SUCCESS) {
        spdlog::critical("Failed to map vertex buffer memory");
        return false;
    }
    memcpy(data, vertices.data(), sizeof(Vertex) * mesh.vertexCount);
    vmaUnmapMemory(allocator, mesh.vertexBufferMemory);

    spdlog::info("Vertex buffer created with {} vertices", mesh.vertexCount);
    return true;
}

void destroyMesh(VulkanMesh& mesh, VulkanDevice& device, VmaAllocator allocator) {
    if (mesh.vertexBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, mesh.vertexBuffer, mesh.vertexBufferMemory);
        mesh.vertexBuffer = VK_NULL_HANDLE;
        mesh.vertexBufferMemory = VK_NULL_HANDLE;
        mesh.vertexCount = 0;
        spdlog::debug("Vertex buffer destroyed");
    }
    // Add index buffer destruction here if implemented
}

// Pipeline Lifecycle
bool createRenderPass(VkRenderPass& renderPass, VulkanDevice& device, VkFormat swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        spdlog::critical("Failed to create render pass");
        return false;
    }

    spdlog::debug("Render pass created successfully");
    return true;
}

void destroyRenderPass(VkRenderPass& renderPass, VulkanDevice& device) {
    if (renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device.logicalDevice, renderPass, nullptr);
        renderPass = VK_NULL_HANDLE;
        spdlog::debug("Render pass destroyed");
    }
}

bool createGraphicsPipeline(
    VulkanPipeline& pipeline,
    VulkanDevice& device,
    VkExtent2D swapChainExtent,
    VkRenderPass compatibleRenderPass,
    const std::string& vertShaderPath,
    const std::string& fragShaderPath
) {
    // Load and create shader modules
    auto vertShaderCode = readFile(vertShaderPath);
    auto fragShaderCode = readFile(fragShaderPath);

    if (vertShaderCode.empty() || fragShaderCode.empty()) {
        spdlog::critical("Failed to read shader files");
        return false;
    }

    VkShaderModule vertShaderModule = createShaderModule(device.logicalDevice, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device.logicalDevice, fragShaderCode);

    if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
        spdlog::critical("Failed to create shader modules");
        return false;
    }

    // Pipeline layout (assuming no dynamic descriptors or push constants for now)
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // No descriptor sets for now
    pipelineLayoutInfo.pushConstantRangeCount = 0; // No push constants for now

    if (vkCreatePipelineLayout(device.logicalDevice, &pipelineLayoutInfo, nullptr, &pipeline.pipelineLayout) != VK_SUCCESS) {
        spdlog::critical("Failed to create pipeline layout");
        return false;
    }

    // Render pass (assuming compatible with swap chain)
    pipeline.renderPass = compatibleRenderPass;

    // Graphics pipeline creation
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; // Vertex and fragment shaders
    pipelineInfo.pStages = nullptr; // To be filled later

    // Fixed function states (simplified)
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Assign shader stages to pipeline
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};

    // Vertex shader stage
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";

    // Fragment shader stage
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";

    pipelineInfo.pStages = shaderStages;    // Vertex input state
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    // For our shader which uses gl_VertexIndex, we don't need any vertex input bindings
    // If we were using an actual vertex buffer with data, we would fill these in
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    // Input assembly state
    pipelineInfo.pInputAssemblyState = &inputAssembly;

    // Viewport and scissor state
    pipelineInfo.pViewportState = &viewportState;

    // Rasterization state
    pipelineInfo.pRasterizationState = &rasterizer;

    // Multisample state
    pipelineInfo.pMultisampleState = &multisampling;

    // Color blend state
    pipelineInfo.pColorBlendState = &colorBlending;

    // Pipeline layout and render pass
    pipelineInfo.layout = pipeline.pipelineLayout;
    pipelineInfo.renderPass = pipeline.renderPass;
    pipelineInfo.subpass = 0; // Assuming single subpass

    if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.graphicsPipeline) != VK_SUCCESS) {
        spdlog::critical("Failed to create graphics pipeline");
        return false;
    }

    // Cleanup shader modules if not managed by the pipeline
    vkDestroyShaderModule(device.logicalDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(device.logicalDevice, fragShaderModule, nullptr);

    spdlog::info("Graphics pipeline created successfully");
    return true;
}

void destroyVulkanPipeline(VulkanPipeline& pipeline, VulkanDevice& device) {
    if (pipeline.graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device.logicalDevice, pipeline.graphicsPipeline, nullptr);
        pipeline.graphicsPipeline = VK_NULL_HANDLE;
        spdlog::debug("Graphics pipeline destroyed");
    }
    if (pipeline.pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device.logicalDevice, pipeline.pipelineLayout, nullptr);
        pipeline.pipelineLayout = VK_NULL_HANDLE;
        spdlog::debug("Pipeline layout destroyed");
    }
    // Render pass destruction is now managed separately
}

// Framebuffer Lifecycle
bool createFramebuffers(VulkanSwapChain& swapChain, VulkanDevice& device, VkRenderPass renderPass) {
    swapChain.framebuffers.resize(swapChain.imageViews.size());

    for (size_t i = 0; i < swapChain.imageViews.size(); i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &swapChain.imageViews[i];
        framebufferInfo.width = swapChain.extent.width;
        framebufferInfo.height = swapChain.extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.logicalDevice, &framebufferInfo, nullptr, &swapChain.framebuffers[i]) != VK_SUCCESS) {
            spdlog::critical("Failed to create framebuffer for swap chain image view {}", i);
            return false;
        }
    }

    spdlog::info("Framebuffers created successfully");
    return true;
}

void destroyFramebuffers(VulkanSwapChain& swapChain, VulkanDevice& device) {
    for (auto framebuffer : swapChain.framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device.logicalDevice, framebuffer, nullptr);
        }
    }
    swapChain.framebuffers.clear();
    spdlog::debug("Framebuffers destroyed");
}

// Command Pool & Buffer Management
bool createCommandPool(VulkanRenderer& renderer) {
    // Create a command pool for the graphics queue
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = renderer.device.graphicsQueueFamilyIndex;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffer to be reset

    if (vkCreateCommandPool(renderer.device.logicalDevice, &poolInfo, nullptr, &renderer.commandPool) != VK_SUCCESS) {
        spdlog::critical("Failed to create command pool");
        return false;
    }

    spdlog::debug("Command pool created successfully");
    return true;
}

bool createCommandBuffers(VulkanRenderer& renderer) {
    // Allocate command buffers from the command pool
    renderer.commandBuffers.resize(renderer.synchronization.maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(renderer.commandBuffers.size());

    if (vkAllocateCommandBuffers(renderer.device.logicalDevice, &allocInfo, renderer.commandBuffers.data()) != VK_SUCCESS) {
        spdlog::critical("Failed to allocate command buffers");
        return false;
    }

    spdlog::debug("Command buffers allocated successfully");
    return true;
}

// Drawing Operations
bool drawFrame(
    VulkanRenderer& renderer,
    VulkanPipeline& activePipeline,
    VulkanMesh& meshToDraw
) {
    // Wait for the previous frame to complete
    vkWaitForFences(renderer.device.logicalDevice, 1, &renderer.synchronization.inFlightFences[renderer.synchronization.currentFrame], VK_TRUE, UINT64_MAX);    // Get the index of the next image to render to
    uint32_t imageIndex;
    // Since we have one semaphore per swap chain image, we can always use semaphore 0 to acquire the next image
    // After acquisition, we'll use the semaphore corresponding to the acquired image index
    VkResult result = vkAcquireNextImageKHR(renderer.device.logicalDevice, renderer.swapChain.handle, UINT64_MAX,
                                           renderer.synchronization.imageAvailableSemaphores[0],
                                           VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        spdlog::warn("Swap chain out of date, recreate swap chain");
        // Handle swap chain recreation (signal a flag, call recreateSwapChain, etc.)
        return false;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::critical("Failed to acquire swap chain image");
        return false;
    }

    // Check if a previous frame is using this image (i.e. there is its fence to wait on)
    if (renderer.synchronization.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(renderer.device.logicalDevice, 1, &renderer.synchronization.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    // Mark the image as now being in use by this frame
    renderer.synchronization.imagesInFlight[imageIndex] = renderer.synchronization.inFlightFences[renderer.synchronization.currentFrame];

    // Reset the fence for the current frame
    vkResetFences(renderer.device.logicalDevice, 1, &renderer.synchronization.inFlightFences[renderer.synchronization.currentFrame]);

    // Reset the command buffer for this frame
    vkResetCommandBuffer(renderer.commandBuffers[renderer.synchronization.currentFrame], 0);

    // Record commands for this frame
    recordCommandBuffer(renderer.commandBuffers[renderer.synchronization.currentFrame], imageIndex, renderer, activePipeline, meshToDraw);    // Submit the command buffer for execution
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;    // Wait for the imageAvailable semaphore that we used to acquire the image (always semaphore 0)
    VkSemaphore waitSemaphores[] = {renderer.synchronization.imageAvailableSemaphores[0]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    
    // Command buffer to submit
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &renderer.commandBuffers[renderer.synchronization.currentFrame];    // Signal the renderFinished semaphore for the specific image
    VkSemaphore signalSemaphores[] = {renderer.synchronization.renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
      // Submit the command buffer
    if (vkQueueSubmit(renderer.device.graphicsQueue, 1, &submitInfo, 
                     renderer.synchronization.inFlightFences[renderer.synchronization.currentFrame]) != VK_SUCCESS) {
        spdlog::critical("Failed to submit draw command buffer");
        return false;
    }

    // Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderer.synchronization.renderFinishedSemaphores[imageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &renderer.swapChain.handle;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(renderer.device.presentQueue, &presentInfo);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        spdlog::warn("Swap chain out of date or suboptimal, recreate swap chain");
        // Handle swap chain recreation
        return false;
    } else if (result != VK_SUCCESS) {
        spdlog::critical("Failed to present swap chain image");
        return false;
    }

    // Move to the next frame
    renderer.synchronization.currentFrame = (renderer.synchronization.currentFrame + 1) % renderer.synchronization.maxFramesInFlight;

    return true;
}

void recordCommandBuffer(
    VkCommandBuffer commandBuffer,
    uint32_t imageIndex,
    VulkanRenderer& renderer,
    VulkanPipeline& activePipeline,
    VulkanMesh& meshToDraw
) {
    // Begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional: VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        spdlog::critical("Failed to begin recording command buffer");
        return;
    }

    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = activePipeline.renderPass;
    renderPassInfo.framebuffer = renderer.swapChain.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderer.swapChain.extent;
    
    // Set clear color to dark gray (RGBA in normalized floats: 0.2, 0.2, 0.2, 1.0)
    VkClearValue clearColor{};
    clearColor.color = {0.2f, 0.2f, 0.2f, 1.0f};
    
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // If we have a valid pipeline and mesh, draw it
    if (activePipeline.graphicsPipeline != VK_NULL_HANDLE && meshToDraw.vertexCount > 0) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline.graphicsPipeline);
        
        VkBuffer vertexBuffers[] = {meshToDraw.vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        
        vkCmdDraw(commandBuffer, meshToDraw.vertexCount, 1, 0, 0);
    }
    
    vkCmdEndRenderPass(commandBuffer);
    
    // End command buffer recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        spdlog::critical("Failed to record command buffer");
        return;
    }

    spdlog::debug("Command buffer recorded successfully for image index {}", imageIndex);
}