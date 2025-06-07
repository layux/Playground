#include "MiniEngine/Graphics/Window.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

using namespace MiniEngine::Graphics;

Window::Window(const std::string& title, uint32_t width, uint32_t height)
    : m_Title(title), m_Width(width), m_Height(height)
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    spdlog::debug("GLFW initialized");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // Allow resizing

    m_Handle = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);

    if (!m_Handle)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    spdlog::info("Window created: {} ({}x{})", m_Title, m_Width, m_Height);
}

Window::~Window()
{
    if (m_Handle)
    {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
        spdlog::debug("GLFW window destroyed");
    }

    glfwTerminate();
    spdlog::debug("GLFW terminated");
}

