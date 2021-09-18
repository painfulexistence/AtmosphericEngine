#include "Framework/Window.hpp"
#include "GUI/ImGui.hpp"

std::map<Window*, OnMouseMoveCallback> Window::onMouseMoveCallbacks = std::map<Window*, OnMouseMoveCallback>();

std::map<Window*, OnMouseEnterCallback> Window::onMouseEnterCallbacks = std::map<Window*, OnMouseEnterCallback>();
    
std::map<Window*, OnMouseLeaveCallback> Window::onMouseLeaveCallbacks = std::map<Window*, OnMouseLeaveCallback>();

std::map<Window*, OnKeyPressCallback> Window::onKeyPressCallbacks = std::map<Window*, OnKeyPressCallback>();

std::map<Window*, OnKeyReleaseCallback> Window::onKeyReleaseCallbacks = std::map<Window*, OnKeyReleaseCallback>();

Window::Window(Framework* fw) : _fw(fw)
{
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

    this->_win = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric", NULL, NULL);
    if (this->_win == nullptr)
        throw std::runtime_error("Failed to initialize window!");
}

Window::~Window()
{
    glfwDestroyWindow(this->_win);
}

void Window::Init()
{
    // Setup GUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    #if USE_VULKAN_DRIVER
    throw std::runtime_error("When using Vulkan, GUI context should be manually handled!");
    #else    
    ImGui_ImplGlfw_InitForOpenGL(this->_win, true); // platform binding
    ImGui_ImplOpenGL3_Init("#version 410"); // renderer binding
    #endif

    // Setup input management
    //glfwSetInputMode(this->_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(this->_win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetCursorPosCallback(this->_win, [](GLFWwindow* win, double x, double y) {
        for (auto kv : Window::onMouseMoveCallbacks)
            kv.second(win, (float)x, (float)y);
    });

    glfwSetCursorEnterCallback(this->_win, [](GLFWwindow* win, int entered) {
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

    glfwSetKeyCallback(this->_win, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
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
}

void Window::SetActive()
{
    #if USE_VULKAN_DRIVER
    // Skip context binding
    #else
    glfwMakeContextCurrent(this->_win);
    #if VSYNC_ON
    glfwSwapInterval(1);
    #endif
    #endif
}

void Window::SwapBuffers()
{
    #if USE_VULKAN_DRIVER
    throw std::runtime_error("When using Vulkan, swapchain should be manually handled!");
    #else
    glfwSwapBuffers(this->_win);
    #endif
}

void Window::PollEvents()
{
    glfwPollEvents(); // Snapshot the keyboard
}

void Window::SetOnMouseMove(OnMouseMoveCallback callback)
{
    onMouseMoveCallbacks[this] = callback;
}

void Window::SetOnMouseMove()
{
    onMouseMoveCallbacks[this] = [](GLFWwindow*, float, float) {};
}

void Window::SetOnMouseEnter(OnMouseEnterCallback callback)
{
    onMouseEnterCallbacks[this] = callback;
}

void Window::SetOnMouseEnter()
{
    onMouseEnterCallbacks[this] = [](GLFWwindow*) {};
}

void Window::SetOnMouseLeave(OnMouseLeaveCallback callback)
{
    onMouseLeaveCallbacks[this] = callback;
}

void Window::SetOnMouseLeave()
{
    onMouseLeaveCallbacks[this] = [](GLFWwindow*) {};
}

void Window::SetOnKeyPress(OnKeyPressCallback callback)
{
    onKeyPressCallbacks[this] = callback;
}

void Window::SetOnKeyPress()
{
    onKeyPressCallbacks[this] = [](GLFWwindow*, int, int, int) {};
}

void Window::SetOnKeyRelease(OnKeyReleaseCallback callback)
{
    onKeyReleaseCallbacks[this] = callback;
}

void Window::SetOnKeyRelease()
{
    onKeyReleaseCallbacks[this] = [](GLFWwindow*, int, int, int) {};
}

glm::vec2 Window::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(this->_win, &x, &y);
    return glm::vec2((float)x, (float)y);
}

bool Window::GetKeyDown(int key)
{
    bool isDown = (glfwGetKey(this->_win, key) == GLFW_PRESS);
    return isDown;
}

bool Window::GetKeyUp(int key)
{
    bool isUp = (glfwGetKey(this->_win, key) == GLFW_RELEASE);
    return isUp;
}

bool Window::IsClosing()
{
    return glfwWindowShouldClose(this->_win);
}