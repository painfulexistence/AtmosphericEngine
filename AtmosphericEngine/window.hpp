#pragma once
#include "globals.hpp"
#include "config.hpp"


struct ImageSize
{
    ImageSize(int width, int height) : width(width), height(height) {};
    int width;
    int height;
};

struct WindowProps
{
    std::string title = INIT_SCREEN_TITLE;
    int width = INIT_SCREEN_WIDTH;
    int height = INIT_SCREEN_HEIGHT;
};

enum class KeyState {
    PRESSED,
    RELEASED,
    HELD,
    UNKNOWN
};

enum class Key {
    SPACE,
    ENTER,
    ESCAPE,
    UP,
    RIGHT,
    LEFT,
    DOWN,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Num0,
    Q,
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P,
    A,
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    Z,
    X,
    C,
    V,
    B,
    N,
    M,
    UNKNOWN
};
const Key FIRST_KEY = Key::SPACE;
const Key LAST_KEY = Key::UNKNOWN;

struct KeyEvent {
    Key key;
    uint64_t time;
};

using MouseMoveCallback = std::function<void(float, float)>;
using MouseEnterCallback = std::function<void()>;
using MouseLeaveCallback = std::function<void()>;
using KeyPressCallback = std::function<void(Key, int)>;
using KeyReleaseCallback = std::function<void(Key, int)>;
using ViewportResizeCallback = std::function<void(int, int)>;
using FramebufferResizeCallback = std::function<void(int, int)>;

using WindowEventCallbackID = uint32_t;

class Window {
private:
    static Window* _instance;

public:
    static Window* Get()
    {
        return _instance;
    }

    Window(WindowProps props = WindowProps());
    ~Window();

    void Init();

    void InitImGui();
    void BeginImGuiFrame();
    void EndImGuiFrame();
    void DeinitImGui();

    void Present();

    void PollEvents();

    void Close();

    WindowEventCallbackID AddMouseMoveCallback(MouseMoveCallback callback);
    WindowEventCallbackID AddMouseEnterCallback(MouseEnterCallback callback);
    WindowEventCallbackID AddMouseLeaveCallback(MouseLeaveCallback callback);
    WindowEventCallbackID AddKeyPressCallback(KeyPressCallback callback);
    WindowEventCallbackID AddKeyReleaseCallback(KeyReleaseCallback callback);
    WindowEventCallbackID AddViewportResizeCallback(ViewportResizeCallback callback);
    WindowEventCallbackID AddFramebufferResizeCallback(FramebufferResizeCallback callback);
    void RemoveMouseMoveCallback(WindowEventCallbackID id);
    void RemoveMouseEnterCallback(WindowEventCallbackID id);
    void RemoveMouseLeaveCallback(WindowEventCallbackID id);
    void RemoveKeyPressCallback(WindowEventCallbackID id);
    void RemoveKeyReleaseCallback(WindowEventCallbackID id);
    void RemoveViewportResizeCallback(WindowEventCallbackID id);
    void RemoveFramebufferResizeCallback(WindowEventCallbackID id);
    void RemoveAllEventCallbacks();

    glm::vec2 GetMousePosition();

    bool GetMouseButtonState();

    // Legacy
    bool GetKeyDown(Key key);
    // Legacy
    bool GetKeyUp(Key key);
    KeyState GetKeyState(Key key);

    std::string GetTitle();
    void SetTitle(const std::string& title);

    float GetTime();
    void SetTime(double time);

    ImageSize GetViewportSize();

    ImageSize GetSize();

    bool IsClosing();

private:
    void* _internal = nullptr;
    WindowEventCallbackID _nextCallbackID = 0;
    std::unordered_map<WindowEventCallbackID, MouseMoveCallback> _mouseMoveCallbacks;
    std::unordered_map<WindowEventCallbackID, MouseEnterCallback> _mouseEnterCallbacks;
    std::unordered_map<WindowEventCallbackID, MouseLeaveCallback> _mouseLeaveCallbacks;
    std::unordered_map<WindowEventCallbackID, KeyPressCallback> _keyPressCallbacks;
    std::unordered_map<WindowEventCallbackID, KeyReleaseCallback> _keyReleaseCallbacks;
    std::unordered_map<WindowEventCallbackID, ViewportResizeCallback> _viewportResizeCallbacks;
    std::unordered_map<WindowEventCallbackID, FramebufferResizeCallback> _framebufferResizeCallbacks;
};