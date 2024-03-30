#include "Application.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//#include <iostream> // Note that IO should only be used for debugging here
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
    Window* win = new Window();
    win->Init();
    this->_window = win; // Multi-window not supported now
}

Application::~Application()
{
    Log("Exiting...");
    for (const auto& go : gameObjects)
        delete go;
    for (const auto& [name, mesh] : Mesh::MeshList)
        delete mesh;
    delete this->_mb;
    delete this->_window;
}

void Application::Run()
{
    Log("Initializing...");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    #if USE_VULKAN_DRIVER
    throw std::runtime_error("When using Vulkan, GUI context should be manually handled!");
    #else
    ImGui_ImplGlfw_InitForOpenGL(this->GetWindow()->GetGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
    #endif
    ImGui::StyleColorsDark();

    console.Init(_mb, this);
    input.Init(_mb, this);
    physics.Init(_mb, this);
    graphics.Init(_mb, this);
    script.Init(_mb, this);
    //ecs.Init(_mb, this);
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
    graphics.colorProgram = ShaderProgram(shaderTable["color"]["vert"], shaderTable["color"]["frag"]);
    graphics.depthTextureProgram = ShaderProgram(shaderTable["depth"]["vert"], shaderTable["depth"]["frag"]);
    graphics.depthCubemapProgram = ShaderProgram(shaderTable["depth_cubemap"]["vert"], shaderTable["depth_cubemap"]["frag"]);
    graphics.hdrProgram = ShaderProgram(shaderTable["hdr"]["vert"], shaderTable["hdr"]["frag"]);
    script.Print("Shaders initialized.");

    const sol::table materialTable = scene["materials"];
    for (const auto& kv : materialTable)
    {
        auto mat = new Material((sol::table)kv.second);
        graphics.materials.push_back(mat);
    }
    script.Print("Materials initialized.");

    const sol::table lightTable = scene["lights"];
    std::vector<GameObject*> lights;
    for (const auto& kv : lightTable)
    {
        auto light = new GameObject();
        ComponentFactory::CreateLight(light, &graphics, LightProps((sol::table)kv.second));
        gameObjects.push_back(light);
        lights.push_back(light);
    }
    mainLight = dynamic_cast<Light*>(lights.at(0)->GetComponent("Light"));

    sol::table cameraTable = scene["cameras"];
    for (const auto& kv : cameraTable)
    {
        auto camera = new GameObject();
        ComponentFactory::CreateCamera(camera, &graphics, CameraProps((sol::table)kv.second));
        gameObjects.push_back(camera);
        cameras.push_back(camera);
    }
    mainCamera = dynamic_cast<Camera*>(cameras.at(0)->GetComponent("Camera"));



    Load();

    float lastFrameTime = GetWindowTime();
    float deltaTime = 0;
    while (!this->_quitted)
    {
        Tick();
        this->_window->PollEvents();
        if (this->_window->IsClosing())
            this->_mb->PostMessage(MessageType::ON_QUIT);

        float currentFrameTime = GetWindowTime();
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        FrameProps currentFrame = FrameProps(this->GetClock(), GetWindowTime(), deltaTime);

        #if SINGLE_THREAD
        #pragma region main_loop_single_thread
        Process(currentFrame);
        SyncTransformWithPhysics();
        Render(currentFrame);
        Draw(currentFrame);
        #pragma endregion
        #else
        #pragma region main_loop_double_thread
        std::thread fork(&Application::Process, this, currentFrame);
        Render(currentFrame);
        Draw(currentFrame);
        fork.join();
        SyncTransformWithPhysics();
        #pragma endregion
        #endif
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Log("Game quitted.");
}

void Application::Quit()
{
    Log("Requested to quit.");
    this->_quitted = true;
}

void Application::Process(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    Update(dt, time);
    //ecs.Process(dt); // Note that most of the entity manipulation logic should be put there
    BroadcastMessages();
    console.Process(dt);
    input.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    graphics.Process(dt); // TODO: Generate command buffers according to entity transforms

    #if SHOW_PROCESS_COST
    Log(fmt::format("Update costs {} ms", (Time() - time) * 1000));
    #endif
}

void Application::Render(const FrameProps& props)
{
    float dt = props.deltaTime;
    float time = GetWindowTime();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    graphics.Render(dt);
    graphics.RenderUI(dt);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Nevertheless, glFinish() can force the GPU process all the commands synchronously.
    glFinish();

    #if SHOW_RENDER_AND_DRAW_COST
    Log(fmt::format("Render & draw cost {} ms", (GetWindowTime() - time) * 1000));
    #endif
}

void Application::Draw(const FrameProps& props)
{
    float time = GetWindowTime();

    this->_window->SwapBuffers();

    #if SHOW_VSYNC_COST
    Log(fmt::format("Vsync cost {} ms", (Time() - time) * 1000));
    #endif
}

void Application::BroadcastMessages()
{
    this->_mb->Process();
}

void Application::SyncTransformWithPhysics()
{
    float time = GetWindowTime();

    //ecs.SyncTransformWithPhysics();
    for (auto go : gameObjects)
    {
        auto impostor = dynamic_cast<Impostor*>(go->GetComponent("Physics"));
        if (impostor == nullptr)
            continue;
        go->SetModelWorldTransform(impostor->GetCenterOfMassWorldTransform());
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

float Application::GetWindowTime()
{
    return this->_window->GetTime();
}

Window* Application::GetWindow()
{
    return this->_window;
}