#pragma once
#include "config.hpp"
#include "globals.hpp"
#include "imgui.h"
#include "console.hpp"
// #include "audio_manager.hpp"
#include "graphics_server.hpp"
#include "physics_server.hpp"
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

struct AppConfig {
    std::string windowTitle = INIT_SCREEN_TITLE;
    int windowWidth = INIT_SCREEN_WIDTH;
    int windowHeight = INIT_SCREEN_HEIGHT;
    bool windowResizable = false;
    bool windowFloating = false;
    bool fullscreen = false;
    bool vsync = true;
    bool enableAudio = true;
    bool enableGraphics3D = true;
    bool enablePhysics3D = true;
    float fixedTimeStep = FIXED_TIME_STEP;
    bool useDefaultTextures = false;
    bool useDefaultShaders = true;
};

using EntityID = uint64_t;

class Application {
public:
    explicit Application(AppConfig config = {});
    ~Application();

    void Run();

    virtual void OnInit() {} // Load resources
    virtual void OnLoad() = 0; // Setup game objects (side effects)
    virtual void OnUpdate(float dt, float time) = 0;
    virtual void OnReload() {} // Reset game objects (side effects clean up and recreate)

    uint64_t GetClock();

    GameObject* GetDefaultGameObject() {
        if (!_defaultGameObject) {
            _defaultGameObject = CreateGameObject();
            _defaultGameObject->SetName("__root__");
        }
        return _defaultGameObject;
    }

protected:
    // These subsystems will be game accessible
    // AudioManager audio;
    GraphicsServer graphics;
    PhysicsServer physics;
    Console console;
    Input input;
    Script script;

    std::vector<Scene> scenes;
    Camera* mainCamera = nullptr;
    Light* mainLight = nullptr;

    void LoadScene(const SceneDef& scene);
    void ReloadScene();

    void Quit();

    std::shared_ptr<Window> GetWindow();

    float GetWindowTime();
    void SetWindowTime(float time);

    std::string GetWindowTitle();
    void SetWindowTitle(const std::string& title);

    template<typename T>
    std::shared_ptr<T> AddSubsystem() {
        static_assert(std::is_base_of<Server, T>::value, "Type T must be a subclass of Server");
        auto subsystem = std::make_shared<T>();
        _subsystems.push_back(subsystem);
        return subsystem;
    }

    GameObject* CreateGameObject(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    GameObject* CreateGameObject(glm::vec2 position, float rotation = 0.0f);

private:
    AppConfig _config;

    std::shared_ptr<Window> _window = nullptr;
    std::vector<std::shared_ptr<Server>> _subsystems;
    bool _initialized = false;

    uint64_t _clock = 0;
    uint16_t _sceneIndex = 0;
    std::optional<SceneDef> _currentSceneDef = std::nullopt;
    std::vector<GameObject*> _entities;
    EntityID _nextEntityID = 0;
    GameObject* _defaultGameObject = nullptr;

    bool _showSystemInfo = false;
    bool _showAppView = true;
    bool _showEngineView = true;
    GameObject* _selectedEntity = nullptr;

    void Update(const FrameData& frame);
    void Render(const FrameData& frame); // TODO: Properly separate rendering and drawing logic if the backend supports command buffering
    void SyncTransformWithPhysics();
};