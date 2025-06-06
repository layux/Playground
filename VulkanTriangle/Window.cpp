#include "Window.hpp"

bool CreateWindow(Window& window) {
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window.handle = glfwCreateWindow(window.width, window.height, window.title.c_str(), nullptr, nullptr);
    
    if (!window.handle) {
        glfwTerminate();
        return false;
    }

    return true;
}

bool DestroyWindow(Window& window) {
    if (window.handle) {
        glfwDestroyWindow(window.handle);
        window.handle = nullptr;
    }
    
    glfwTerminate();
    return true;
}