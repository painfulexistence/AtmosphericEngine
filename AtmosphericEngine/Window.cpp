#include "Window.hpp"
#include <GLFW/glfw3.h>

std::map<Window*, OnMouseMoveCallback> Window::onMouseMoveCallbacks = std::map<Window*, OnMouseMoveCallback>();

std::map<Window*, OnMouseEnterCallback> Window::onMouseEnterCallbacks = std::map<Window*, OnMouseEnterCallback>();
    
std::map<Window*, OnMouseLeaveCallback> Window::onMouseLeaveCallbacks = std::map<Window*, OnMouseLeaveCallback>();

std::map<Window*, OnKeyPressCallback> Window::onKeyPressCallbacks = std::map<Window*, OnKeyPressCallback>();

std::map<Window*, OnKeyReleaseCallback> Window::onKeyReleaseCallbacks = std::map<Window*, OnKeyReleaseCallback>();

std::map<Window*, OnViewportResizeCallback> Window::onViewportResizeCallbacks = std::map<Window*, OnViewportResizeCallback>();

std::map<Window*, OnFramebufferResizeCallback> Window::onFramebufferResizeCallbacks = std::map<Window*, OnFramebufferResizeCallback>();

Window::Window(WindowProps props)
{
    glfwSetErrorCallback([](int code, const char* msg) {
        throw std::runtime_error(fmt::format("Error occurred: {}\n", msg));
    });
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    #if USE_VULKAN_DRIVER 
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (!glfwVulkanSupported())
        throw std::runtime_error("Vulkan API not supported");
    #else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    #endif

    this->_glfwWindow = glfwCreateWindow(props.width, props.height, props.title.c_str(), NULL, NULL);
    if (this->_glfwWindow == nullptr)
        throw std::runtime_error("Failed to create window!");
}

Window::~Window()
{
    glfwDestroyWindow(this->_glfwWindow);
    
    glfwTerminate();
}

void Window::Init()
{
    #if USE_VULKAN_DRIVER
    // Skip context binding
    #else
    glfwMakeContextCurrent(this->_glfwWindow);
    #if VSYNC_ON
    glfwSwapInterval(1);
    #else
    glfwSwapInterval(0);
    #endif
    #endif

    // Setup input management
    //glfwSetInputMode(this->_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(this->_glfwWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // TODO: debug this section by adding callbacks, and then creating windows
    // Setup event listeners on this window -- any event will be broadcasted to all windows which listens
    #pragma region window_event_listeners
    glfwSetCursorPosCallback(this->_glfwWindow, [](GLFWwindow* win, double x, double y) {
        for (auto kv : Window::onMouseMoveCallbacks)
            kv.second(win, (float)x, (float)y);
    });
    glfwSetCursorEnterCallback(this->_glfwWindow, [](GLFWwindow* win, int entered) {
        if (entered)
        {
            for (auto kv : Window::onMouseEnterCallbacks)
                kv.second(win);
        }
        else
        {
            for (auto kv : Window::onMouseLeaveCallbacks)
                kv.second(win);
        }
    });
    glfwSetKeyCallback(this->_glfwWindow, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS)
        {
            for (auto kv : onKeyPressCallbacks)
                kv.second(win, key, scancode, mods);
        }
        else
        {
            for (auto kv : onKeyReleaseCallbacks)
                kv.second(win, key, scancode, mods);
        }
    });
    glfwSetWindowSizeCallback(this->_glfwWindow, [](GLFWwindow* win, int width, int height) {
        for (auto kv : onViewportResizeCallbacks)
            kv.second(win, width, height);
    });
    glfwSetFramebufferSizeCallback(this->_glfwWindow, [](GLFWwindow* win, int width, int height) {
        for (auto kv : onFramebufferResizeCallbacks)
            kv.second(win, width, height);
    });
    #pragma endregion

    // Default event listeners
    SetOnMouseMove([](float x, float y) {
        //fmt::print("-- Mouse moved to ({},{})\n", x, y);
    });
    SetOnViewportResize([](int width, int height) {
        //fmt::print("-- Viewport resized to {}X{}\n", width, height); 
    });
}

void Window::SwapBuffers()
{
    #if USE_VULKAN_DRIVER
    throw std::runtime_error("When using Vulkan, swapchain should be manually handled!");
    #else
    glfwSwapBuffers(this->_glfwWindow);
    #endif
}

void Window::PollEvents()
{
    glfwPollEvents(); // Snapshot the keyboard
}

void Window::SetOnMouseMove(OnWindowMouseMoveCallback callback)
{
    onMouseMoveCallbacks[this] = [this, callback](GLFWwindow* win, float x, float y) {
        if (this->_glfwWindow == win) 
            callback(x, y); 
    };
}

void Window::SetOnMouseMove()
{
    onMouseMoveCallbacks[this] = [](GLFWwindow*, float, float) {};
}

void Window::SetOnMouseEnter(OnWindowMouseEnterCallback callback)
{
    onMouseEnterCallbacks[this] = [this, callback](GLFWwindow* win) {
        if (this->_glfwWindow == win) 
            callback();
    };
}

void Window::SetOnMouseEnter()
{
    onMouseEnterCallbacks[this] = [](GLFWwindow*) {};
}

void Window::SetOnMouseLeave(OnWindowMouseLeaveCallback callback)
{
    onMouseLeaveCallbacks[this] = [this, callback](GLFWwindow* win) {
        if (this->_glfwWindow == win) 
            callback();
    };
}

void Window::SetOnMouseLeave()
{
    onMouseLeaveCallbacks[this] = [](GLFWwindow*) {};
}

void Window::SetOnKeyPress(OnWindowKeyPressCallback callback)
{
    onKeyPressCallbacks[this] = [this, callback](GLFWwindow* win, int key, int scancode, int mods) {
        if (this->_glfwWindow == win)
            callback(key, scancode, mods);
    };
}

void Window::SetOnKeyPress()
{
    onKeyPressCallbacks[this] = [](GLFWwindow*, int, int, int) {};
}

void Window::SetOnKeyRelease(OnWindowKeyReleaseCallback callback)
{
    onKeyReleaseCallbacks[this] = [this, callback](GLFWwindow* win, int key, int scancode, int mods) {
        if (this->_glfwWindow == win)
            callback(key, scancode, mods);
    };
}

void Window::SetOnKeyRelease()
{
    onKeyReleaseCallbacks[this] = [](GLFWwindow*, int, int, int) {};
}

void Window::SetOnViewportResize(OnWindowViewportResizeCallback callback)
{
    onViewportResizeCallbacks[this] = [this, callback](GLFWwindow* win, int width, int height) {
        if (this->_glfwWindow == win)
            callback(width, height);
    };
}

void Window::SetOnViewportResize()
{
    onViewportResizeCallbacks[this] = [](GLFWwindow*, int, int) {};

}

void Window::SetOnFramebufferResize(OnWindowFramebufferResizeCallback callback)
{
    onFramebufferResizeCallbacks[this] = [this, callback](GLFWwindow* win, int width, int height) {
        if (this->_glfwWindow == win)
            callback(width, height);
    };
}

void Window::SetOnFramebufferResize()
{
    onFramebufferResizeCallbacks[this] =  [](GLFWwindow*, int, int) {};
}

glm::vec2 Window::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(this->_glfwWindow, &x, &y);
    return glm::vec2((float)x, (float)y);
}

bool Window::GetKeyDown(int key)
{
    bool isDown = (glfwGetKey(this->_glfwWindow, key) == GLFW_PRESS);
    return isDown;
}

bool Window::GetKeyUp(int key)
{
    bool isUp = (glfwGetKey(this->_glfwWindow, key) == GLFW_RELEASE);
    return isUp;
}

float Window::GetTime()
{
    return (float)glfwGetTime(); // Note that glfwGetTime() only starts to calculate time after the window is created;
}

GLFWwindow* Window::GetGLFWWindow()
{
    return this->_glfwWindow;
}

ImageSize Window::GetViewportSize()
{
    int width, height;
    glfwGetWindowSize(this->_glfwWindow, &width, &height);
    return ImageSize(width, height);
}

ImageSize Window::GetFramebufferSize()
{
    int width, height;
    glfwGetFramebufferSize(this->_glfwWindow, &width, &height);
    return ImageSize(width, height);
}

bool Window::IsClosing()
{
    return glfwWindowShouldClose(this->_glfwWindow);
}