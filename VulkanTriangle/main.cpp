#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <array>
#include <vector>

struct Window
{
    std::string title;
    uint32_t width;
    uint32_t height;
    GLFWwindow* handle = nullptr;
};

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

struct VulkanSwapchain
{
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

struct VulkanSynchronization
{
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;
    size_t                   currentFrame      = 0;
    size_t                   maxFramesInFlight = 2;
    bool                     frameInProgress   = false;
};

struct VulkanCommands
{
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers; // one per frame in-flight
};

struct VulkanRendererContext
{
    VulkanDevice device;
    VulkanSwapchain swapchain;
    VulkanSynchronization sync;
    VulkanCommands commands;
};

struct VulkanPipeline
{
    VkShaderModule vertexShader = VK_NULL_HANDLE;
    VkShaderModule fragmentShader = VK_NULL_HANDLE;
    std::array<VkVertexInputBindingDescription, 1> vertexInputBindingDescriptions = {};
    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescriptions = {};
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
};

struct VulkanMesh
{
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation vertexBufferAllocation = VK_NULL_HANDLE;
    uint32_t vertexCount = 0;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VmaAllocation indexBufferAllocation = VK_NULL_HANDLE;
    uint32_t indexCount = 0;
};