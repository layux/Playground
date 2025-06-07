#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <string>

namespace MiniEngine::Graphics
{
    class Window
    {
    public:
        Window(const std::string& title, uint32_t width, uint32_t height);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        bool ShouldClose() const
        {
            return glfwWindowShouldClose(m_Handle);
        }

        void Update();

    private:
        std::string m_Title;
        uint32_t m_Width;
        uint32_t m_Height;
        GLFWwindow* m_Handle = nullptr;
    };
}