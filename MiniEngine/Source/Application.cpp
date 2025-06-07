#include "MiniEngine/Application.hpp"

using namespace MiniEngine;

Application::Application()
    : m_Window(new Graphics::Window("MiniEngine", 1280, 720))
{
}

Application::~Application()
{
    delete m_Window;
}

void Application::Run()
{
    while (!m_Window->ShouldClose())
    {
        m_Window->Update();
    }
}