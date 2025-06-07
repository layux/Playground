#pragma once

#include "MiniEngine/Graphics/Window.hpp"

namespace MiniEngine
{
    class Application
    {
    public:
        Application();
        ~Application();

        void Run();

    private:
        Graphics::Window* m_Window;
    };
}