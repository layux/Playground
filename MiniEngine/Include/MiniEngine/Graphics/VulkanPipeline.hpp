#pragma once

#include "MiniEngine/Graphics/Pipeline.hpp"

namespace MiniEngine::Graphics
{
    class VulkanPipeline : public Pipeline
    {
    public:
        VulkanPipeline();
        ~VulkanPipeline() override;

        VulkanPipeline(const VulkanPipeline&) = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;
        VulkanPipeline(VulkanPipeline&&) = delete;
        VulkanPipeline& operator=(VulkanPipeline&&) = delete;

    private:
        // Add Vulkan-specific members here
    };
} // namespace MiniEngine::Graphics