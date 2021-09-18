#pragma once
#include "Globals.hpp"
#include "Framework/Framework.hpp"

#define OnMouseMoveCallback std::function<void(GLFWwindow*, float, float)>
#define OnMouseEnterCallback std::function<void(GLFWwindow*)>
#define OnMouseLeaveCallback std::function<void(GLFWwindow*)>
#define OnKeyPressCallback std::function<void(GLFWwindow*, int, int, int)>
#define OnKeyReleaseCallback std::function<void(GLFWwindow*, int, int, int)>

class Window
{
public:
    static std::map<Window*, OnMouseMoveCallback> onMouseMoveCallbacks;

    static std::map<Window*, OnMouseEnterCallback> onMouseEnterCallbacks;
    
    static std::map<Window*, OnMouseLeaveCallback> onMouseLeaveCallbacks;

    static  std::map<Window*, OnKeyPressCallback> onKeyPressCallbacks;

    static  std::map<Window*, OnKeyReleaseCallback> onKeyReleaseCallbacks;

    Window(Framework* fw);

    ~Window();
    
    void Init();

    void SetActive();

    void SwapBuffers();
    
    void PollEvents();

    void AddEventListener();

    void SetOnMouseMove(OnMouseMoveCallback callback);
    
    void SetOnMouseMove();
    
    void SetOnMouseEnter(OnMouseEnterCallback callback);
    
    void SetOnMouseEnter();
    
    void SetOnMouseLeave(OnMouseLeaveCallback callback);
    
    void SetOnMouseLeave();
    
    void SetOnKeyPress(OnKeyPressCallback callback);
    
    void SetOnKeyPress();

    void SetOnKeyRelease(OnKeyReleaseCallback callback);
    
    void SetOnKeyRelease();

    glm::vec2 GetMousePosition();

    bool GetKeyDown(int key);

    bool GetKeyUp(int key);

    bool IsClosing();
    
private:
    Framework* _fw;

    GLFWwindow* _win = nullptr;
};