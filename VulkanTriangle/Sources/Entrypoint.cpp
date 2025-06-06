#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

struct VulkanDevice;
struct VulkanSwapChain;
struct VulkanSynchronization;

struct VulkanRenderer
{
	VulkanDevice*                device          = nullptr;
	VulkanSwapChain*             swapChain       = nullptr;
	VulkanSynchronization*       synchronization = nullptr;
	VkCommandPool                commandPool     = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> commandBuffers;
};

struct VulkanDevice
{
	VkInstance               instance                 = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger           = VK_NULL_HANDLE;
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
	VkSurfaceKHR               surface              = VK_NULL_HANDLE;
	VkSwapchainKHR             swapChain            = VK_NULL_HANDLE;
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

/// Function declarations
int main(int argc, char** argv)
{

}