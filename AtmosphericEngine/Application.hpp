#pragma once
#include "Globals.hpp"
#include "imgui.h"
#include "MessageBus.hpp"
#include "Window.hpp"
#include "GraphicsServer.hpp"
#include "PhysicsServer.hpp"
#include "Console.hpp"
#include "Input.hpp"
#include "Script.hpp"
#include "GameObject.hpp"

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

    void Quit();

    virtual void Load() = 0;

    virtual void Update(float dt, float time) = 0;
    
    uint64_t GetClock();

    float GetWindowTime();

    Window* GetWindow();

protected:
    // These subsystems will be game accessible
    GraphicsServer graphics;
    PhysicsServer physics;
    Console console;
    Input input;
    Script script;
    std::vector<GameObject*> gameObjects;

private:
    bool _initialized = false;
    bool _quitted = false;

    MessageBus* _mb = new MessageBus(this);
    Window* _window = nullptr;
    uint64_t _clock = 0;

    void Log(std::string message);

    void Tick();

    void Process(const FrameProps& frame);

    void Render(const FrameProps& frame); // TODO: Properly separate rendering and drawing logic if the backend supports command buffering

    void Draw(const FrameProps& frame);

    void BroadcastMessages();

    void SyncTransformWithPhysics();
};