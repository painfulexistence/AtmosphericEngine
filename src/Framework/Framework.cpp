#include "Framework/Framework.hpp"
#include "Framework/Window.hpp"

Framework::Framework()
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

Framework::~Framework()
{
    delete this->_activeWindow;
    glfwTerminate();
}

void Framework::Tick()
{
    this->_clock++;
}

uint64_t Framework::GetClock()
{
    return this->_clock;
}

float Framework::GetTime()
{
    float time = (float)glfwGetTime(); // Note that glfwGetTime() only starts to calculate time after the window is created
    return time;
}

Window* Framework::GetActiveWindow()
{
    return this->_activeWindow;
}