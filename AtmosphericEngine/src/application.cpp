#include "application.hpp"
#include "animator_2d.hpp"
#include "asset_manager.hpp"
#include "camera_component.hpp"
#include "camera_controller_2d.hpp"
#include "component_registry.hpp"
#include "editor_layer.hpp"
#include "game_layer.hpp"
#include "game_object.hpp"
#include "job_system.hpp"
#include "light_component.hpp"
#include "mesh_component.hpp"
#include "rigidbody_2d_component.hpp"
#include "rigidbody_component.hpp"
#include "rmlui_manager.hpp"
#include "scene.hpp"
#include "shape_renderer_component.hpp"
#include "sprite_3d_component.hpp"
#include "sprite_component.hpp"
#include "terrain_component.hpp"
#include "transform_component.hpp"
#include "window.hpp"
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

Application::Application(AppConfig config) : _config(config) {
    // setbuf(stdout, NULL); // Cancel output stream buffering so that output can be seen immediately

    _window = std::make_shared<Window>(WindowProps{
      .title = config.windowTitle,
      .width = config.windowWidth,
      .height = config.windowHeight,
      .resizable = config.windowResizable,
      .floating = config.windowFloating,
      .fullscreen = config.fullscreen,
      .vsync = config.vsync,
    });// Multi-window not supported now
    _window->Init();
    _window->InitImGui();

    PushLayer(new GameLayer(this));
    PushLayer(new EditorLayer(this));

    RegisterComponents();
}

void Application::RegisterComponents() {
    ComponentRegistry::Register("TransformComponent",
      [](GameObject* o, const void* /*p*/) -> Component* {
          return new TransformComponent(o, {0,0,0}, {0,0,0}, {1,1,1});
      });

    ComponentRegistry::Register("SpriteComponent",
      [](GameObject* o, const void* p) -> Component* {
          const SpriteProps defaults{};
          return new SpriteComponent(o, p ? *static_cast<const SpriteProps*>(p) : defaults);
      });

    ComponentRegistry::Register("CameraComponent",
      [](GameObject* o, const void* p) -> Component* {
          const CameraProps defaults{};
          return new CameraComponent(o, p ? *static_cast<const CameraProps*>(p) : defaults);
      });

    ComponentRegistry::Register("LightComponent",
      [](GameObject* o, const void* p) -> Component* {
          const LightProps defaults{LightType::Directional, {0.1f,0.1f,0.1f}, {1,1,1}, {1,1,1}, {0,-1,0}, {1,0,0}, 1.0f, false};
          return new LightComponent(o, p ? *static_cast<const LightProps*>(p) : defaults);
      });

    ComponentRegistry::Register("RigidbodyComponent",
      [](GameObject* o, const void* p) -> Component* {
          const RigidbodyProps defaults{};
          const auto& props = p ? *static_cast<const RigidbodyProps*>(p) : defaults;
          return new RigidbodyComponent(o, props.shape, props.mass, props.linearFactor, props.angularFactor);
      });

    ComponentRegistry::Register("Rigidbody2DComponent",
      [](GameObject* o, const void* p) -> Component* {
          const Rigidbody2DProps defaults{};
          return new Rigidbody2DComponent(o, p ? *static_cast<const Rigidbody2DProps*>(p) : defaults);
      });

    ComponentRegistry::Register("ShapeRendererComponent",
      [](GameObject* o, const void* p) -> Component* {
          const ShapeRendererProps defaults{};
          return new ShapeRendererComponent(o, p ? *static_cast<const ShapeRendererProps*>(p) : defaults);
      });

    ComponentRegistry::Register("Animator2D",
      [](GameObject* o, const void* /*p*/) -> Component* {
          return new Animator2D(o);
      });

    ComponentRegistry::Register("CameraController2D",
      [](GameObject* o, const void* /*p*/) -> Component* {
          return new CameraController2D(o);
      });
}

Application::~Application() {
    ENGINE_LOG("Exiting...");
    _window->DeinitImGui();

    for (const auto& go : _entities)
        delete go;

    for (auto* layer : _layers) {
        layer->OnDetach();
        delete layer;
    }
}

void Application::Run() {
#ifdef TRACY_ENABLE
    TracyNoop;
    tracy::InitCallstack();
#endif
    console.Init(this);
    input.Init(this);
    audio.Init(this);
    graphics.Init(this);
    physics.Init(this);// Note that physics debug drawer is dependent on graphics server
    physics2D.Init(this);
    for (auto& subsystem : _subsystems) {
        subsystem->Init(this);
    }
    ENGINE_LOG("Subsystems initialized.");

    auto windowSize = _window->GetFramebufferSize();
    RmlUiManager::Get()->Initialize(windowSize.width, windowSize.height, graphics.renderer);

    OnInit();

    OnLoad();

    _window->MainLoop([this](float currTime, float deltaTime) {
        FrameData currFrame = { GetClock(), currTime, deltaTime };
#ifdef TRACY_ENABLE
        FrameMark;
#endif
#if SINGLE_THREAD || defined(__EMSCRIPTEN__)
        // Emscripten: no pthreads in this build — update and render serially.
        Update(currFrame);
        Render(currFrame);
#else
        std::thread fork(&Application::Update, this, currFrame);
        Render(currFrame);
        fork.join();
#endif
        _clock++;
    });

    RmlUiManager::Get()->Shutdown();
}

void Application::PushLayer(Layer* layer) {
    _layers.push_back(layer);
    layer->OnAttach();
}

void Application::LoadScene(const SceneDef& scene) {
    ENGINE_LOG("Loading scene...");

    if (_config.useDefaultTextures) {
        AssetManager::Get().LoadDefaultTextures();
    }
    AssetManager::Get().LoadTextures(scene.textures);
    ENGINE_LOG("Textures created.");

    if (_config.useDefaultShaders) {
        AssetManager::Get().LoadDefaultShaders();
    }
    AssetManager::Get().LoadShaders(scene.shaders);
    ENGINE_LOG("Shaders created.");

    AssetManager::Get().LoadMaterials(scene.materials);
    ENGINE_LOG("Materials created.");

    for (const auto& go : scene.gameObjects) {
        auto entity = CreateGameObject(go.position, go.rotation, go.scale);
        entity->SetName(go.name);
        if (go.camera.has_value()) {
            entity->AddComponent<CameraComponent>(go.camera.value());
        }
        if (go.light.has_value()) {
            entity->AddComponent<LightComponent>(go.light.value());
        }
    }
    ENGINE_LOG("Game objects created.");

    mainCamera = graphics.GetMainCamera();
    mainLight = graphics.GetMainLight();

    _currentSceneDef = scene;
}

void Application::ReloadScene() {
    // TODO: making sure all resources are cleaned up before loading
    AssetManager::Get().Clear();
    graphics.cameras.clear();
    graphics.directionalLights.clear();
    graphics.pointLights.clear();

    audio.StopAll();
    physics.Reset();

    for (auto e : _entities) {
        delete e;
    }
    _entities.clear();

    // TODO: reload scene
    if (_currentSceneDef.has_value()) {
        OnLoad();
        // LoadScene(_currentSceneDef.value());
    }
}

void Application::Quit() {
    ENGINE_LOG("Requested to quit.");
    _window->Close();
}

void Application::Update(const FrameData& props) {
#ifdef TRACY_ENABLE
    ZoneScopedN("Application::Update");
#endif
    float dt = props.deltaTime;

    OnUpdate(dt, GetWindowTime());

    // ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    console.Process(dt);
    input.Process(dt);
    audio.Process(dt);
    physics.Process(dt);// TODO: Update only every entity's physics transform
    physics2D.Process(dt);
    graphics.Process(dt);
    for (auto& subsystem : _subsystems) {
        subsystem->Process(dt);
    }

#if SHOW_PROCESS_COST
    ENGINE_LOG(fmt::format("Update costs {} ms", (GetWindowTime() - time) * 1000));
#endif

    for (auto layer : _layers) {
        layer->OnUpdate(dt);
    }

    float time = GetWindowTime();

    for (auto go : _entities) {
        auto impostor = go->GetComponent<RigidbodyComponent>();
        if (impostor == nullptr) continue;
        if (impostor->IsKinematic()) continue;
        go->SyncObjectTransform(impostor->GetWorldTransform());
    }
}

void Application::Render(const FrameData& props) {
#ifdef TRACY_ENABLE
    ZoneScopedN("Application::Render");
#endif
    float dt = props.deltaTime;
    float time = GetWindowTime();

    for (auto* layer : _layers) {
        layer->OnRender(dt);
    }

#if SHOW_RENDER_AND_DRAW_COST
    ENGINE_LOG(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
#endif
}

void Application::SyncTransformWithPhysics() {
}

uint64_t Application::GetClock() {
    return this->_clock;
}

std::shared_ptr<Window> Application::GetWindow() {
    return this->_window;
}

float Application::GetWindowTime() {
    return this->_window->GetTime();
}

void Application::SetWindowTime(float time) {
    this->_window->SetTime(time);
}

std::string Application::GetWindowTitle() {
    return this->_window->GetTitle();
}

void Application::SetWindowTitle(const std::string& title) {
    this->_window->SetTitle(title);
}

GameObject* Application::CreateGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    auto e = new GameObject(this, position, rotation, scale);
    e->SetName(fmt::format("entity #{}", _nextEntityID++));
    _entities.push_back(e);
    return e;
}

GameObject* Application::CreateGameObject(glm::vec2 position, float angle) {
    auto e =
      new GameObject(this, glm::vec3(position.x, position.y, 0.0f), glm::vec3(0.0f, 0.0f, angle), glm::vec3(1.0f));
    e->SetName(fmt::format("entity #{}", _nextEntityID++));
    _entities.push_back(e);
    return e;
}