#include "application.hpp"
#include "job_system.hpp"
#include "window.hpp"
#include "game_object.hpp"
#include "scene.hpp"
#include "impostor.hpp"
#include "drawable_2d.hpp"

Application::Application(AppConfig config) : _config(config)
{
    // setbuf(stdout, NULL); // Cancel output stream buffering so that output can be seen immediately

    _window = std::make_shared<Window>(WindowProps {
        .title = config.windowTitle,
        .width = config.windowWidth,
        .height = config.windowHeight,
        .resizable = config.windowResizable,
        .floating = config.windowFloating,
        .fullscreen = config.fullscreen,
        .vsync = config.vsync,
    }); // Multi-window not supported now
    _window->Init();
    _window->InitImGui();

    JobSystem::Get()->Init();
}

Application::~Application()
{
    ENGINE_LOG("Exiting...");
    _window->DeinitImGui();

    for (const auto& go : _entities)
        delete go;

    for (auto* layer : _layers) {
        layer->OnDetach();
        delete layer;
    }
}

void Application::Run()
{
    console.Init(this);
    input.Init(this);
    // audio.Init(this);
    graphics.Init(this);
    physics.Init(this); // Note that physics debug drawer is dependent on graphics server
    script.Init(this);
    for (auto& subsystem : _subsystems) {
        subsystem->Init(this);
    }
    ENGINE_LOG("Subsystems initialized.");

    OnInit();

    OnLoad();

    _window->MainLoop([this](float currTime, float deltaTime) {
        FrameData currFrame = { GetClock(), currTime, deltaTime };
#if SINGLE_THREAD
        Update(currFrame);
        Render(currFrame);
#else
        std::thread fork(&Application::Update, this, currFrame);
        Render(currFrame);
        fork.join();
#endif
        _clock++;
    });
}

void Application::PushLayer(Layer* layer) {
    _layers.push_back(layer);
    layer->OnAttach();
}

void Application::LoadScene(const SceneDef& scene) {
    ENGINE_LOG("Loading scene...");

    if (_config.useDefaultTextures) {
        graphics.LoadDefaultTextures();
    }
    graphics.LoadTextures(scene.textures);
    ENGINE_LOG("Textures created.");

    if (_config.useDefaultShaders) {
        graphics.LoadDefaultShaders();
    }
    graphics.LoadShaders(scene.shaders);
    ENGINE_LOG("Shaders created.");

    graphics.LoadMaterials(scene.materials);
    ENGINE_LOG("Materials created.");

    for (const auto& go : scene.gameObjects) {
        auto entity = CreateGameObject(go.position, go.rotation, go.scale);
        entity->SetName(go.name);
        if (go.camera.has_value()) {
            entity->AddCamera(go.camera.value());
        }
        if (go.light.has_value()) {
            entity->AddLight(go.light.value());
        }
    }
    ENGINE_LOG("Game objects created.");

    mainCamera = graphics.GetMainCamera();
    mainLight = graphics.GetMainLight();

    _currentSceneDef = scene;
}

void Application::ReloadScene() {
    // TODO: clean up resources to avoid memory leaks
    graphics.textures.clear();
    graphics.materials.clear();
    graphics.meshes.clear();
    graphics.cameras.clear();
    graphics.directionalLights.clear();
    graphics.pointLights.clear();

    // audio.StopAll();

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

void Application::Quit()
{
    ENGINE_LOG("Requested to quit.");
    _window->Close();
}

void Application::Update(const FrameData& props)
{
    float dt = props.deltaTime;

    OnUpdate(dt, GetWindowTime());

    //ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    console.Process(dt);
    input.Process(dt);
    // audio.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    for (auto& subsystem : _subsystems) {
        subsystem->Process(dt);
    }

#if SHOW_PROCESS_COST
    ENGINE_LOG(fmt::format("Update costs {} ms", (GetWindowTime() - time) * 1000));
#endif

    float time = GetWindowTime();

    for (auto go : _entities) {
        auto impostor = go->GetComponent<Impostor>();
        if (impostor == nullptr) continue;
        go->SyncObjectTransform(impostor->GetWorldTransform());
    }
}

void Application::Render(const FrameData& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Process(dt); // TODO: Generate command buffers according to entity transforms
    // graphics.Render(dt);
    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    // glFinish();

    for (auto* layer : _layers) {
        layer->OnRender();
    }

#if SHOW_RENDER_AND_DRAW_COST
    ENGINE_LOG(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
#endif
}

void Application::SyncTransformWithPhysics()
{
}

uint64_t Application::GetClock()
{
    return this->_clock;
}

std::shared_ptr<Window> Application::GetWindow()
{
    return this->_window;
}

float Application::GetWindowTime()
{
    return this->_window->GetTime();
}

void Application::SetWindowTime(float time)
{
    this->_window->SetTime(time);
}

std::string Application::GetWindowTitle()
{
    return this->_window->GetTitle();
}

void Application::SetWindowTitle(const std::string& title)
{
    this->_window->SetTitle(title);
}

GameObject* Application::CreateGameObject(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    auto e = new GameObject(&graphics, &physics, position, rotation, scale);
    e->SetName(fmt::format("entity #{}", _nextEntityID++));
    _entities.push_back(e);
    return e;
}

GameObject* Application::CreateGameObject(glm::vec2 position, float angle)
{
    auto e = new GameObject(&graphics, &physics, glm::vec3(position.x, position.y, 0.0f), glm::vec3(0.0f, 0.0f, angle), glm::vec3(1.0f));
    e->SetName(fmt::format("entity #{}", _nextEntityID++));
    _entities.push_back(e);
    return e;
}