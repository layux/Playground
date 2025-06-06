#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

struct Window
{
    std::string title;
    uint32_t width;
    uint32_t height;
    GLFWwindow* handle = nullptr;
};

bool CreateWindow(Window& window);
bool DestroyWindow(Window& window);