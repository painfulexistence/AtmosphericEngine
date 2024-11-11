#pragma once
#include "globals.hpp"
#include "config.hpp"

#define OnMouseMoveCallback std::function<void(GLFWwindow*, float, float)>
#define OnMouseEnterCallback std::function<void(GLFWwindow*)>
#define OnMouseLeaveCallback std::function<void(GLFWwindow*)>
#define OnKeyPressCallback std::function<void(GLFWwindow*, int, int, int)>
#define OnKeyReleaseCallback std::function<void(GLFWwindow*, int, int, int)>
#define OnViewportResizeCallback std::function<void(GLFWwindow*, int, int)>
#define OnFramebufferResizeCallback std::function<void(GLFWwindow*, int, int)>

#define OnWindowMouseMoveCallback std::function<void(float, float)>
#define OnWindowMouseEnterCallback std::function<void()>
#define OnWindowMouseLeaveCallback std::function<void()>
#define OnWindowKeyPressCallback std::function<void(int, int, int)>
#define OnWindowKeyReleaseCallback std::function<void(int, int, int)>
#define OnWindowViewportResizeCallback std::function<void(int, int)>
#define OnWindowResizeCallback std::function<void(int, int)>

class GLFWwindow;

struct ImageSize
{
    ImageSize(int width, int height) : x(width), y(height) {};
    int x;
    int y;
};

struct WindowProps
{
    WindowProps(std::string title = INIT_SCREEN_TITLE, int width = INIT_SCREEN_WIDTH, int height = INIT_SCREEN_HEIGHT)
    {
        this->title = title;
        this->width = width;
        this->height = height;
    };
    std::string title;
    int width;
    int height;
};

class Window
{
public:
    static Window* Get()
    {
        return _instance;
    }

    static std::map<Window*, OnMouseMoveCallback> onMouseMoveCallbacks;

    static std::map<Window*, OnMouseEnterCallback> onMouseEnterCallbacks;

    static std::map<Window*, OnMouseLeaveCallback> onMouseLeaveCallbacks;

    static  std::map<Window*, OnKeyPressCallback> onKeyPressCallbacks;

    static  std::map<Window*, OnKeyReleaseCallback> onKeyReleaseCallbacks;

    static  std::map<Window*, OnViewportResizeCallback> onViewportResizeCallbacks;

    static  std::map<Window*, OnFramebufferResizeCallback> onResizeCallbacks;

    Window(WindowProps props = WindowProps());

    ~Window();

    void Init();

    void SwapBuffers();

    void PollEvents();

    void AddEventListener();

    void SetOnMouseMove(OnWindowMouseMoveCallback callback);

    void SetOnMouseMove();

    void SetOnMouseEnter(OnWindowMouseEnterCallback callback);

    void SetOnMouseEnter();

    void SetOnMouseLeave(OnWindowMouseLeaveCallback callback);

    void SetOnMouseLeave();

    void SetOnKeyPress(OnWindowKeyPressCallback callback);

    void SetOnKeyPress();

    void SetOnKeyRelease(OnWindowKeyReleaseCallback callback);

    void SetOnKeyRelease();

    void SetOnViewportResize(OnWindowViewportResizeCallback callback);

    void SetOnViewportResize();

    void SetOnResize(OnWindowResizeCallback callback);

    void SetOnResize();

    glm::vec2 GetMousePosition();

    bool GetKeyDown(int key);

    bool GetKeyUp(int key);

    std::string GetTitle();

    void SetTitle(const std::string& title);

    float GetTime();

    void SetTime(double time);

    GLFWwindow* GetGLFWWindow();

    ImageSize GetViewportSize();

    ImageSize GetSize();

    bool IsClosing();

private:
    static Window* _instance;

    GLFWwindow* _glfwWindow = nullptr;
};