#include "window.hpp"
#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

Window* Window::_instance = nullptr;

std::map<Window*, OnMouseMoveCallback> Window::onMouseMoveCallbacks = std::map<Window*, OnMouseMoveCallback>();

std::map<Window*, OnMouseEnterCallback> Window::onMouseEnterCallbacks = std::map<Window*, OnMouseEnterCallback>();

std::map<Window*, OnMouseLeaveCallback> Window::onMouseLeaveCallbacks = std::map<Window*, OnMouseLeaveCallback>();

std::map<Window*, OnKeyPressCallback> Window::onKeyPressCallbacks = std::map<Window*, OnKeyPressCallback>();

std::map<Window*, OnKeyReleaseCallback> Window::onKeyReleaseCallbacks = std::map<Window*, OnKeyReleaseCallback>();

std::map<Window*, OnViewportResizeCallback> Window::onViewportResizeCallbacks = std::map<Window*, OnViewportResizeCallback>();

std::map<Window*, OnFramebufferResizeCallback> Window::onResizeCallbacks = std::map<Window*, OnFramebufferResizeCallback>();

Window::Window(WindowProps props)
{
    if (_instance != nullptr)
        throw std::runtime_error("Window is already initialized!");

    glfwSetErrorCallback([](int code, const char* msg) {
        throw std::runtime_error(fmt::format("Error occurred: {}\n", msg));
    });
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    this->_internal = glfwCreateWindow(props.width, props.height, props.title.c_str(), NULL, NULL);
    if (this->_internal == nullptr)
        throw std::runtime_error("Failed to create window!");

    _instance = this;
}

Window::~Window()
{
    glfwDestroyWindow(static_cast<GLFWwindow*>(_internal));

    glfwTerminate();
}

void Window::Init()
{
    glfwMakeContextCurrent(static_cast<GLFWwindow*>(_internal));
#if VSYNC_ON
    glfwSwapInterval(1);
#else
    glfwSwapInterval(0);
#endif
    // Setup input management
    //glfwSetInputMode(this->_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(static_cast<GLFWwindow*>(_internal), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // TODO: debug this section by adding callbacks, and then creating windows
    // Setup event listeners on this window -- any event will be broadcasted to all windows which listens
    #pragma region window_event_listeners
    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(_internal), [](GLFWwindow* win, double x, double y) {
        for (auto kv : Window::onMouseMoveCallbacks)
            kv.second(win, (float)x, (float)y);
    });
    glfwSetCursorEnterCallback(static_cast<GLFWwindow*>(_internal), [](GLFWwindow* win, int entered) {
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
    glfwSetKeyCallback(static_cast<GLFWwindow*>(_internal), [](GLFWwindow* win, int key, int scancode, int action, int mods) {
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
    glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(_internal), [](GLFWwindow* win, int width, int height) {
        for (auto kv : onViewportResizeCallbacks)
            kv.second(win, width, height);
    });
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(_internal), [](GLFWwindow* win, int width, int height) {
        for (auto kv : onResizeCallbacks)
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

void Window::InitImGui()
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(_internal), true);
    ImGui_ImplOpenGL3_Init("#version 410");

    ImGui::StyleColorsDark();
}

void Window::BeginImGuiFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Window::EndImGuiFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::DeinitImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Window::Present()
{
    glfwSwapBuffers(static_cast<GLFWwindow*>(_internal));
}

void Window::PollEvents()
{
    glfwPollEvents(); // Snapshot the keyboard
}

void Window::SetOnMouseMove(OnWindowMouseMoveCallback callback)
{
    onMouseMoveCallbacks[this] = [this, callback](GLFWwindow* win, float x, float y) {
        if (this->_internal == win)
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
        if (this->_internal == win)
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
        if (this->_internal == win)
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
        if (this->_internal == win)
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
        if (this->_internal == win)
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
        if (this->_internal == win)
            callback(width, height);
    };
}

void Window::SetOnViewportResize()
{
    onViewportResizeCallbacks[this] = [](GLFWwindow*, int, int) {};

}

void Window::SetOnResize(OnWindowResizeCallback callback)
{
    onResizeCallbacks[this] = [this, callback](GLFWwindow* win, int width, int height) {
        if (this->_internal == win)
            callback(width, height);
    };
}

void Window::SetOnResize()
{
    onResizeCallbacks[this] =  [](GLFWwindow*, int, int) {};
}

glm::vec2 Window::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(static_cast<GLFWwindow*>(_internal), &x, &y);
    return glm::vec2((float)x, (float)y);
}

bool Window::GetKeyDown(int key)
{
    bool isDown = (glfwGetKey(static_cast<GLFWwindow*>(_internal), key) == GLFW_PRESS);
    return isDown;
}

bool Window::GetKeyUp(int key)
{
    bool isUp = (glfwGetKey(static_cast<GLFWwindow*>(_internal), key) == GLFW_RELEASE);
    return isUp;
}

std::string Window::GetTitle()
{
    return std::string(glfwGetWindowTitle(static_cast<GLFWwindow*>(_internal)));
}

void Window::SetTitle(const std::string& title)
{
    glfwSetWindowTitle(static_cast<GLFWwindow*>(_internal), title.c_str());
}

float Window::GetTime()
{
    return (float)glfwGetTime(); // Note that glfwGetTime() only starts to calculate time after the window is created;
}

void Window::SetTime(double time)
{
    glfwSetTime(time);
}

ImageSize Window::GetViewportSize()
{
    int width, height;
    glfwGetWindowSize(static_cast<GLFWwindow*>(_internal), &width, &height);
    return ImageSize(width, height);
}

ImageSize Window::GetSize()
{
    int width, height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(_internal), &width, &height);
    return ImageSize(width, height);
}

bool Window::IsClosing()
{
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(_internal));
}