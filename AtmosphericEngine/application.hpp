#pragma once
#include "globals.hpp"
#include "imgui.h"
#include "audio_manager.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"
#include "console.hpp"
#include "input.hpp"
#include "script.hpp"

class Window;

class Scene;

class GameObject;

struct FrameData
{
    FrameData(uint64_t number, float time, float deltaTime)
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

    virtual void OnLoad() = 0;

    virtual void OnUpdate(float dt, float time) = 0;

    GameObject* GetDefaultGameObject() {
        if (!_defaultGameObject) {
            _defaultGameObject = CreateGameObject();
            _defaultGameObject->SetName("__root__");
        }
        return _defaultGameObject;
    }

protected:
    // These subsystems will be game accessible
    AudioManager audio;
    GraphicsServer graphics;
    PhysicsServer physics;
    Console console;
    Input input;
    Script script;

    std::vector<Scene> scenes;
    Camera* mainCamera = nullptr;
    Light* mainLight = nullptr;

    void LoadScene(SceneDef& scene);

    void Quit();

    std::shared_ptr<Window> GetWindow();

    float GetWindowTime();

    void SetWindowTime(float time);

    std::string GetWindowTitle();

    void SetWindowTitle(const std::string& title);

    void AddSubsystem(std::shared_ptr<Server> subsystem);

    uint64_t GetClock();

    GameObject* CreateGameObject(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

private:
    std::vector<std::shared_ptr<Server>> _subsystems;
    bool _initialized = false;

    std::shared_ptr<Window> _window = nullptr;
    uint64_t _clock = 0;
    uint16_t _sceneIndex = 0;
    std::vector<GameObject*> _entities;
    GameObject* _defaultGameObject = nullptr;

    bool _showSystemInfo = false;
    bool _showAppView = true;
    bool _showEngineView = true;
    GameObject* _selectedEntity = nullptr;

    void Log(std::string message);

    void Tick();

    void Update(const FrameData& frame);

    void Render(const FrameData& frame); // TODO: Properly separate rendering and drawing logic if the backend supports command buffering

    void PresentWindow(const FrameData& frame);

    void SyncTransformWithPhysics();
};