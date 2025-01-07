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
}

void Application::Run()
{
    console.Init(this);
    input.Init(this);
    audio.Init(this);
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
    audio.Process(dt);
    script.Process(dt);
    physics.Process(dt); // TODO: Update only every entity's physics transform
    for (auto& subsystem : _subsystems) {
        subsystem->Process(dt);
    }

#if SHOW_PROCESS_COST
    ENGINE_LOG(fmt::format("Update costs {} ms", (GetWindowTime() - time) * 1000));
#endif
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

    _window->BeginImGuiFrame();

    if (ImGui::BeginMainMenuBar()) {
        // if (ImGui::BeginMenu("File")) {
        //     if (ImGui::MenuItem("New Scene")) { }
        //     if (ImGui::MenuItem("Open Scene")) { }
        //     if (ImGui::MenuItem("Save Scene")) { }
        //     ImGui::EndMenu();
        // }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("System Info", nullptr, &_showSystemInfo);
            ImGui::MenuItem("Engine", nullptr, &_showEngineView);
            ImGui::MenuItem("Application", nullptr, &_showAppView);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (_showSystemInfo) {
        ImGui::Begin("System Information");
        {
            ImGui::Text("OpenGL: %s", glGetString(GL_VERSION));
            ImGui::Text("GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
            ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
            auto [wx, wy] = _window->GetSize();
            ImGui::Text("Window size: %dx%d", wx, wy);
            auto [fx, fy] = _window->GetFramebufferSize();
            ImGui::Text("Framebuffer size: %dx%d", fx, fy);

            GLint depth, stencil;
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);
            ImGui::Text("Depth bits: %d", depth);
            ImGui::Text("Stencil bits: %d", stencil);

            GLint maxVertUniforms, maxFragUniforms;
            glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertUniforms);
            glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragUniforms);
            ImGui::Text("Max vertex uniforms: %d bytes", maxVertUniforms / 4);
            ImGui::Text("Max fragment uniforms: %d bytes", maxFragUniforms / 4);

            GLint maxVertUniBlocks, maxFragUniBlocks;
            glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertUniBlocks);
            glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragUniBlocks);
            ImGui::Text("Max vertex uniform blocks: %d", maxVertUniBlocks);
            ImGui::Text("Max fragment uniform blocks: %d", maxFragUniBlocks);

            GLint maxElementIndices, maxElementVertices;
            glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementIndices);
            glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementVertices);
            ImGui::Text("Max element indices: %d", maxElementIndices);
            ImGui::Text("Max element vertices: %d", maxElementVertices);
        }
        ImGui::End();
    }

    if (_showAppView) {
        ImGui::Begin("Application");
        {
            ImGui::BeginChild("Scene", ImVec2(200, 400), true);
            ImGui::Text("Scene (%d entities)", (uint32_t)_entities.size());
            ImGui::Separator();
            ImGui::BeginGroup();
            for (auto& entity : _entities) {
                bool selected = entity == _selectedEntity;
                if (ImGui::Selectable(entity->GetName().c_str(), selected)) {
                    _selectedEntity = entity;
                }
            }
            ImGui::EndGroup();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Entity", ImVec2(300, 400), true);
            ImGui::Text("Entity");
            ImGui::Separator();
            if (_selectedEntity) {
                ImGui::Text("Name: %s", _selectedEntity->GetName().c_str());
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    glm::vec3 pos = _selectedEntity->GetPosition();
                    glm::vec3 rot = _selectedEntity->GetRotation();
                    glm::vec3 scale = _selectedEntity->GetScale();
                    if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                        _selectedEntity->SetPosition(pos);
                    if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f))
                        _selectedEntity->SetRotation(rot);
                    if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
                        _selectedEntity->SetScale(scale);
                }

                auto impostor = static_cast<Impostor*>(_selectedEntity->GetComponent("Physics"));
                if (impostor != nullptr) {
                    if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
                        glm::vec3 vel = impostor->GetLinearVelocity();
                        ImGui::Text("Velocity: %.3f, %.3f, %.3f", vel.x, vel.y, vel.z);
                    }
                }
                auto light = static_cast<Light*>(_selectedEntity->GetComponent("Light"));
                if (light != nullptr) {
                    if (light->type == LightType::Directional) {
                        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::DragFloat3("direction", &light->direction.x);
                            ImGui::ColorEdit3("diffuse", &light->diffuse.r);
                            ImGui::ColorEdit3("spcular", &light->specular.r);
                            ImGui::ColorEdit3("ambient", &light->ambient.r);
                            ImGui::DragFloat("intensity", &light->intensity);
                            ImGui::Checkbox("castShadow", &light->castShadow);
                        }
                    } else if (light->type == LightType::Point) {
                        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen)) {
                            ImGui::DragFloat3("attenuation", &light->attenuation.x);
                            ImGui::ColorEdit3("diffuse", &light->diffuse.r);
                            ImGui::ColorEdit3("spcular", &light->specular.r);
                            ImGui::ColorEdit3("ambient", &light->ambient.r);
                            ImGui::DragFloat("intensity", &light->intensity);
                            ImGui::Checkbox("castShadow", &light->castShadow);
                        }
                    }
                }
                auto camera = static_cast<Camera*>(_selectedEntity->GetComponent("Camera"));
                if (camera != nullptr) {
                    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                    }
                }
                auto renderable = static_cast<Renderable*>(_selectedEntity->GetComponent("Drawable"));
                if (renderable != nullptr) {
                    if (ImGui::CollapsingHeader("Renderable", ImGuiTreeNodeFlags_DefaultOpen)) {
                        auto mat = renderable->GetMaterial();
                        ImGui::SliderInt("Base map ID", &mat->baseMap, -1, graphics.textures.size() - 1);
                        ImGui::SliderInt("Normal map ID", &mat->normalMap, -1, graphics.textures.size() - 1);
                        ImGui::SliderInt("AO map ID", &mat->aoMap, -1, graphics.textures.size() - 1);
                        ImGui::SliderInt("Roughness map ID", &mat->roughnessMap, -1, graphics.textures.size() - 1);
                        ImGui::SliderInt("Metallic map ID", &mat->metallicMap, -1, graphics.textures.size() - 1);
                        ImGui::SliderInt("Height map ID", &mat->heightMap, -1, graphics.textures.size() - 1);
                        ImGui::ColorEdit3("Diffuse", &mat->diffuse.r);
                        ImGui::ColorEdit3("Specular", &mat->specular.r);
                        ImGui::ColorEdit3("Ambient", &mat->ambient.r);
                        ImGui::DragFloat("Shininess", &mat->shininess, 0.0f, 1.0f);
                        ImGui::Checkbox("Cull face enabled", &mat->cullFaceEnabled);
                    }
                }
                auto drawable2D = static_cast<Drawable2D*>(_selectedEntity->GetComponent("Drawable2D"));
                if (drawable2D != nullptr) {
                    if (ImGui::CollapsingHeader("Drawable2D", ImGuiTreeNodeFlags_DefaultOpen)) {
                        glm::vec2 size = drawable2D->GetSize();
                        glm::vec2 pivot = drawable2D->GetPivot();
                        glm::vec4 color = drawable2D->GetColor();
                        uint8_t textureID = drawable2D->GetTextureID();
                        if (ImGui::DragFloat2("Size:", &size.x, 0.001f, 9999.999f)) {
                            drawable2D->SetSize(size);
                        }
                        if (ImGui::DragFloat2("Pivot:", &pivot.x, 0.0f, 1.0f)) {
                            drawable2D->SetPivot(pivot);
                        }
                        if (ImGui::ColorEdit4("Color", &color.r)) {
                            drawable2D->SetColor(color);
                        }
                        uint8_t minTexIndex = 0, maxTexIndex = graphics.canvasTextures.size() - 1;
                        if (ImGui::SliderScalar("Texture ID", ImGuiDataType_U8, &textureID, &minTexIndex, &maxTexIndex)) {
                            drawable2D->SetTextureID(textureID);
                        }
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (_showEngineView) {
        ImGui::Begin("Engine Subsystems");
        {
            console.DrawImGui(dt);
            input.DrawImGui(dt);
            audio.DrawImGui(dt);
            graphics.DrawImGui(dt);
            physics.DrawImGui(dt);
            for (auto subsystem : _subsystems) {
                subsystem->DrawImGui(dt);
            }
        }
        ImGui::End();
    }

    _window->EndImGuiFrame();

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