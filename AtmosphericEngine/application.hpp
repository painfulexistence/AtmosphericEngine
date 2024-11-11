#pragma once
#include "globals.hpp"
#include "imgui.h"
#include "message_bus.hpp"
#include "window.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"
#include "console.hpp"
#include "input.hpp"
#include "script.hpp"
#include "game_object.hpp"
#include "component_factory.hpp"

class Scene;

struct FrameProps
{
    FrameProps(uint64_t number, float time, float deltaTime)
    {
        this->number = number;
        this->time = time;
        this->deltaTime = deltaTime;
    };
    uint64_t number;
    float time;
    float deltaTime;
};

class Application
{
public:
    Application();

    ~Application();

    void Run();

    virtual void Load() = 0;

    virtual void Update(float dt, float time) = 0;

protected:
    // These subsystems will be game accessible
    GraphicsServer graphics;
    PhysicsServer physics;
    Console console;
    Input input;
    Script script;
    std::vector<Scene> scenes;
    std::vector<GameObject*> cameras;
    Camera* mainCamera = nullptr;
    Light* mainLight = nullptr;

    void Quit();

    uint64_t GetClock();

    Window* GetWindow();

    float GetWindowTime();

    void SetWindowTime(float time);

    std::string GetWindowTitle();

    void SetWindowTitle(const std::string& title);

    GameObject* CreateGameObject();

private:
    bool _initialized = false;
    bool _quitted = false;

    Window* _window = nullptr;
    uint64_t _clock = 0;
    uint16_t _sceneIndex = 0;
    std::vector<GameObject*> _entities;

    void Log(std::string message);

    void Tick();

    void Process(const FrameProps& frame);

    void Render(const FrameProps& frame); // TODO: Properly separate rendering and drawing logic if the backend supports command buffering

    void Draw(const FrameProps& frame);

    void SyncTransformWithPhysics();
};