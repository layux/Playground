#pragma once

#include <vulkan/vulkan.h>

#include <array>

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