#pragma once

#include "MiniEngine/Graphics/Renderer.hpp"

namespace MiniEngine::Graphics
{
    class VulkanRenderer : public Renderer
    {
    public:
        VulkanRenderer();
        ~VulkanRenderer() override;
    };
} // namespace MiniEngine::Graphics