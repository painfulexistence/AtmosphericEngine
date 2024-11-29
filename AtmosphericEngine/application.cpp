#include "application.hpp"
#include "window.hpp"
#include "game_object.hpp"
#include "scene.hpp"
#include "impostor.hpp"

using namespace std;

static glm::mat4 ConvertPhysicalMatrix(const btTransform& trans)
{
    btScalar mat[16] = {0.0f};
    trans.getOpenGLMatrix(mat);

    return glm::mat4(
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15]
    );
}

Application::Application()
{
    Log("Launching...");
    _window = std::make_shared<Window>();; // Multi-window not supported now
    _window->Init();
    _window->InitImGui();
}

Application::~Application()
{
    Log("Exiting...");
    _window->DeinitImGui();
    for (const auto& go : _entities)
        delete go;
}

void Application::Run()
{
    Log("Initializing...");

    console.Init(this);
    input.Init(this);
    graphics.Init(this);
    physics.Init(this); // Note that physics debug drawer is dependent on graphics server
    script.Init(this);
    for (auto& subsystem : _subsystems) {
        subsystem->Init(this);
    }
    this->_initialized = true;

    Log("Loading data...");
    const sol::table scene = script.GetData(std::string("scene"));

    const sol::table textureTable = scene["textures"];
    std::vector<std::string> paths;
    for (const auto& kv : textureTable)
    {
        sol::table tex = (sol::table)kv.second;
        paths.push_back((std::string)tex["path"]);
    }
    graphics.LoadTextures(paths);
    script.Print("Textures loaded.");

    const sol::table shaderTable = scene["shaders"];
    graphics.LoadColorShader(ShaderProgram(shaderTable["color"]["vert"], shaderTable["color"]["frag"]));
    graphics.LoadDebugShader(ShaderProgram(shaderTable["debug_line"]["vert"], shaderTable["debug_line"]["frag"]));
    graphics.LoadDepthShader(ShaderProgram(shaderTable["depth"]["vert"], shaderTable["depth"]["frag"]));
    graphics.LoadDepthCubemapShader(ShaderProgram(shaderTable["depth_cubemap"]["vert"], shaderTable["depth_cubemap"]["frag"]));
    graphics.LoadTerrainShader(ShaderProgram(shaderTable["terrain"]["vert"], shaderTable["terrain"]["frag"], shaderTable["terrain"]["tesc"], shaderTable["terrain"]["tese"]));
    graphics.LoadPostProcessShader(ShaderProgram(shaderTable["hdr"]["vert"], shaderTable["hdr"]["frag"]));
    script.Print("Shaders initialized.");

    const sol::table materialTable = scene["materials"];
    for (const auto& kv : materialTable)
    {
        sol::table data = kv.second;
        auto mat = new Material {
            .baseMap = (int)data.get_or("baseMapId", -1),
            .normalMap = (int)data.get_or("normalMapId", -1),
            .aoMap = (int)data.get_or("aoMapId", -1),
            .roughnessMap = (int)data.get_or("roughtnessMapId", -1),
            .metallicMap = (int)data.get_or("metallicMapId", -1),
            .heightMap = (int)data.get_or("heightMapId", -1),
            .diffuse = glm::vec3(data["diffuse"][1], data["diffuse"][2], data["diffuse"][3]),
            .specular = glm::vec3(data["specular"][1], data["specular"][2], data["specular"][3]),
            .ambient = glm::vec3(data["ambient"][1], data["ambient"][2], data["ambient"][3]),
            .shininess = (float)data.get_or("shininess", 0.25)
        };
        graphics.materials.push_back(mat);
    }
    script.Print("Materials initialized.");

    const sol::table lightTable = scene["lights"];
    for (const auto& kv : lightTable)
    {
        sol::table data = kv.second;
        LightProps props = {
            .type = static_cast<LightType>(data.get_or("type", 1)),
            .position = glm::vec3(
                data["position"][1],
                data["position"][2],
                data["position"][3]
            ),
            .direction = glm::vec3(
                data["direction"][1],
                data["direction"][2],
                data["direction"][3]
            ),
            .ambient = glm::vec3(
                data["ambient"][1],
                data["ambient"][2],
                data["ambient"][3]
            ),
            .diffuse = glm::vec3(
                data["diffuse"][1],
                data["diffuse"][2],
                data["diffuse"][3]
            ),
            .specular = glm::vec3(
                data["specular"][1],
                data["specular"][2],
                data["specular"][3]
            ),
            .attenuation = glm::vec3(
                data["attenuation"][1],
                data["attenuation"][2],
                data["attenuation"][3]
            ),
            .intensity = (float)data.get_or("intensity", 1.0),
            .castShadow = (bool)data.get_or("castShadow", 0)
        };
        auto light = CreateGameObject();
        light->AddLight(props);
    }
    mainLight = graphics.GetMainLight();

    sol::table cameraTable = scene["cameras"];
    for (const auto& kv : cameraTable)
    {
        sol::table data = kv.second;
        CameraProps props = {
            .isOrthographic = false,
            .perspective = {
                .fieldOfView = (float)data.get_or("field_of_view", glm::radians(60.f)),
                .aspectRatio = (float)data.get_or("aspect_ratio", 4.f / 3.f),
                .nearClip = (float)data.get_or("near_clip_plane", 0.1f),
                .farClip = (float)data.get_or("far_clip_plane", 1000.0f),
            },
            .verticalAngle = (float)data.get_or("vertical_angle", 0),
            .horizontalAngle = (float)data.get_or("horizontal_angle", 0),
            .eyeOffset = glm::vec3(
                (float)data.get_or("eye_offset.x", 0),
                (float)data.get_or("eye_offset.y", 0),
                (float)data.get_or("eye_offset.z", 0)
            )
        };
        auto go = CreateGameObject();
        go->AddCamera(props);
    }
    mainCamera = graphics.GetMainCamera();

    OnLoad();

    float lastTime = GetWindowTime();
    float deltaTime = 0;
    while (!_window->IsClosing())
    {
        _window->PollEvents();

        float currentTime = GetWindowTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        FrameData currentFrame = { GetClock(), GetWindowTime(), deltaTime };
#if SINGLE_THREAD
        Update(currentFrame);
        SyncTransformWithPhysics();
        Render(currentFrame);
        PresentWindow(currentFrame);
#else
        std::thread fork(&Application::Process, this, currentFrame);
        Render(currentFrame);
        PresentWindow(currentFrame);
        fork.join();
        SyncTransformWithPhysics();
#endif

        Tick();
    }
    Log("Game quitted.");
}

void Application::Quit()
{
    Log("Requested to quit.");
    _window->Close();
}

void Application::Update(const FrameData& props)
{
    float dt = props.deltaTime;

    OnUpdate(dt, GetWindowTime());

    //ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    console.Process(dt);
    input.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    for (auto& subsystem : _subsystems) {
        subsystem->Process(dt);
    }

    #if SHOW_PROCESS_COST
    Log(fmt::format("Update costs {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::Render(const FrameData& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    _window->BeginImGuiFrame();
    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Process(dt); // TODO: Generate command buffers according to entity transforms
    // graphics.Render(dt);
    graphics.RenderUI(dt);
    for (auto subsystem : _subsystems) {
        subsystem->DrawImGui(dt);
    }
    _window->EndImGuiFrame();

    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    glFinish();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::PresentWindow(const FrameData& props)
{
    float time = GetWindowTime();

    this->_window->Present();

    #if SHOW_VSYNC_COST
    Log(fmt::format("Vsync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::SyncTransformWithPhysics()
{
    float time = GetWindowTime();

    //ecs.SyncTransformWithPhysics();
    for (auto go : _entities)
    {
        auto impostor = dynamic_cast<Impostor*>(go->GetComponent("Physics"));
        if (impostor == nullptr)
            continue;
        go->SyncObjectTransform(impostor->GetWorldTransform());
    }

    #if SHOW_SYNC_COST
    Log(fmt::format("Sync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::Log(std::string message)
{
    #if RUNTIME_LOG_ON
        fmt::print("[Engine] {}\n", message);
    #endif
}

void Application::Tick()
{
    this->_clock++;
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
    _entities.push_back(e);
    return e;
}