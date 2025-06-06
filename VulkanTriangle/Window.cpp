#include "Window.hpp"

#include <spdlog/spdlog.h>

bool CreateWindow(Window& window) {
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    spdlog::debug("GLFW initialized successfully.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window.handle = glfwCreateWindow(window.width, window.height, window.title.c_str(), nullptr, nullptr);
    
    if (!window.handle) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    spdlog::info("Window created successfully: {} ({}x{})", window.title, window.width, window.height);

    return true;
}

bool DestroyWindow(Window& window) {
    if (window.handle) {
        glfwDestroyWindow(window.handle);
        window.handle = nullptr;

        spdlog::debug("GLFW window has been destroyed.");
    }
    
    glfwTerminate();
    spdlog::info("Window destroyed successfully.");
    return true;
}