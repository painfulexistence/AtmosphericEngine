#include "Framework/Application.hpp"
#include "Framework/Window.hpp"

Application::Application()
{
    glfwSetErrorCallback([](int code, const char* msg) {
        throw std::runtime_error(fmt::format("Error occurred: {}\n", msg));
    });
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    Window* win = new Window(this);
    win->Init();
    win->SetActive();
    this->_activeWindow = win;
}

Application::~Application()
{
    delete this->_activeWindow;
    glfwTerminate();
}

void Application::Tick()
{
    this->_clock++;
}

uint64_t Application::GetClock()
{
    return this->_clock;
}

float Application::GetTime()
{
    float time = (float)glfwGetTime(); // Note that glfwGetTime() only starts to calculate time after the window is created
    return time;
}

Window* Application::GetActiveWindow()
{
    return this->_activeWindow;
}