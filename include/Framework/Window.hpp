#pragma once
#include "Globals.hpp"
#include "Framework/Application.hpp"

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
#define OnWindowFramebufferResizeCallback std::function<void(int, int)>

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
    static std::map<Window*, OnMouseMoveCallback> onMouseMoveCallbacks;

    static std::map<Window*, OnMouseEnterCallback> onMouseEnterCallbacks;
    
    static std::map<Window*, OnMouseLeaveCallback> onMouseLeaveCallbacks;

    static  std::map<Window*, OnKeyPressCallback> onKeyPressCallbacks;

    static  std::map<Window*, OnKeyReleaseCallback> onKeyReleaseCallbacks;

    static  std::map<Window*, OnViewportResizeCallback> onViewportResizeCallbacks;

    static  std::map<Window*, OnFramebufferResizeCallback> onFramebufferResizeCallbacks;

    Window(Application* app, WindowProps props = WindowProps());

    ~Window();
    
    void Init();

    void SetActive();

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

    void SetOnFramebufferResize(OnWindowFramebufferResizeCallback callback);

    void SetOnFramebufferResize();

    glm::vec2 GetMousePosition();

    bool GetKeyDown(int key);

    bool GetKeyUp(int key);

    ImageSize GetViewportSize();

    ImageSize GetFramebufferSize();

    bool IsClosing();
    
private:
    Application* _app;
    GLFWwindow* _win = nullptr;
};