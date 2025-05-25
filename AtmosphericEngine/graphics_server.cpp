#include "graphics_server.hpp"
#include "stb_image.h"
#include "asset_manager.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "renderable.hpp"
#include "game_object.hpp"
#include "application.hpp"
#include "window.hpp"
#include "job_system.hpp"

#include <cstddef>

GraphicsServer* GraphicsServer::_instance = nullptr;

GraphicsServer::GraphicsServer()
{
    if (_instance != nullptr)
        throw std::runtime_error("GraphicsServer is already initialized!");

    _instance = this;
}

GraphicsServer::~GraphicsServer()
{
    for (const auto& [name, mesh] : _namedMeshes)
        delete mesh;
    for (auto& mat : materials)
        delete mat;
    glDeleteTextures(textures.size(), textures.data());

    glDeleteVertexArrays(1, &debugVAO);
    glDeleteBuffers(1, &debugVBO);
    glDeleteBuffers(1, &screenVAO);
    glDeleteBuffers(1, &screenVBO);
}

void GraphicsServer::Init(Application* app)
{
    Server::Init(app);

    stbi_set_flip_vertically_on_load(true);

#ifndef __EMSCRIPTEN__
    // Note that OpenGL extensions must NOT be initialzed before the window creation
    if (gladLoadGLLoader((GLADloadproc)Window::GetProcAddress()) <= 0)
        throw std::runtime_error("Failed to initialize OpenGL!");
#endif

#ifndef __EMSCRIPTEN__
    glPrimitiveRestartIndex(0xFFFF);
#endif

#ifndef __EMSCRIPTEN__
    glPatchParameteri(GL_PATCH_VERTICES, 4);
#endif

    glLineWidth(2.0f);

    glCullFace(GL_BACK);
#if MSAA_ON
#ifndef __EMSCRIPTEN__
    glEnable(GL_MULTISAMPLE);
#endif
#endif

    auto window = Window::Get();
    auto [width, height] = window->GetFramebufferSize();
    CreateFBOs();
    CreateRTs(RenderTargetProps { width, height });
    window->AddFramebufferResizeCallback([this, window](int newWidth, int newHeight) {
        DestroyRTs();
        CreateRTs(RenderTargetProps { newWidth, newHeight });
    });

    CreateDebugVAO();
    CreateCanvasVAO();
    CreateScreenVAO();

    canvasDrawList.reserve(1 << 16);
    debugLines.reserve(1 << 16);

    defaultCamera = new Camera(app->GetDefaultGameObject(), {
        .isOrthographic = false,
        .perspective = {
            .fieldOfView = 45.0f,
            .aspectRatio = 4.0f / 3.0f,
            .nearClip = 0.1f,
            .farClip = 1000.0f
        },
        .verticalAngle = 0.0f,
        .horizontalAngle = 0.0f,
        .eyeOffset = glm::vec3(0.0f)
    });

    defaultLight = new Light(app->GetDefaultGameObject(), {
        .type = LightType::Directional,
        .ambient = glm::vec3(1.0f, 1.0f, 1.0f),
        .diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
        .direction = glm::vec3(0.0f, -1.0f, 0.0f),
        .intensity = 1.0f,
        .castShadow = false
    });
}

void GraphicsServer::Process(float dt)
{
    // Note that draw calls are asynchronous, which means they return immediately.
    // So the drawing time can only be calculated along with the image presenting.
    Render(dt);
}

void GraphicsServer::Render(float dt)
{
    auxLightCount = std::min(MAX_OMNI_LIGHTS, (int)pointLights.size());

    // TODO: Put the logic of generating command buffers here
    // Setup
    _meshInstanceMap.clear();
    for (auto r : renderables) {
        if (!r->gameObject->isActive)
            continue;

        Mesh* mesh = r->GetMesh();
        Material* material = r->GetMaterial();

        InstanceData instanceData = {
            .modelMatrix = r->gameObject->GetTransform()
        };
        _meshInstanceMap[mesh].push_back(instanceData);
    }

    for (auto d : canvasDrawables) {
        if (!d->gameObject->isActive)
            continue;

        glm::vec2 pos = glm::vec2(d->gameObject->GetPosition());
        float angle = d->gameObject->GetRotation().z;
        glm::vec2 scale = glm::vec2(d->gameObject->GetScale());
        glm::vec2 size = d->GetSize() * scale;
        glm::vec2 pivot = d->GetPivot();

        PushCanvasQuad(
            pos.x,
            pos.y,
            size.x,
            size.y,
            angle,
            pivot.x,
            pivot.y,
            d->GetColor(),
            static_cast<int>(d->GetTextureID())
        );
    }

    ShadowPass(dt);

    if (_currRenderPath == RenderPath::Forward) {
        ForwardPass(dt);
        MSAAResolvePass(dt);
    } else {
        GeometryPass(dt);
        LightingPass(dt);
        MSAAResolvePass(dt); // TODO: remove this
    }

    CanvasPass(dt);
    if (postProcessEnabled) {
        PostProcessPass(dt);
    }
}

void GraphicsServer::DrawImGui(float dt)
{
    if (ImGui::CollapsingHeader("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Frame rate: %.3f ms/frame (%.1f FPS)", 1000.0f * dt, 1.0f / dt);
        ImGui::Text("Average frame rate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::ColorEdit3("Clear color", (float*)&clearColor);
        if (ImGui::Button("Post-processing")) {
            postProcessEnabled = !postProcessEnabled;
        }

        ImGui::Separator();

        if (ImGui::TreeNode("Cameras")) {
            for (auto c : cameras) {
                ImGui::Text("%s (camera)", c->gameObject->GetName().c_str());
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Lights")) {
            for (auto l : directionalLights) {
                const std::string& name = l->gameObject->GetName();
                if (ImGui::TreeNode(name.c_str(), "%s (light)", name.c_str())) {
                    ImGui::Text("Direction: %.3f, %.3f, %.3f", l->direction.x, l->direction.y, l->direction.z);
                    ImGui::Text("Ambient: %.3f, %.3f, %.3f", l->ambient.x, l->ambient.y, l->ambient.z);
                    ImGui::Text("Diffuse: %.3f, %.3f, %.3f", l->diffuse.x, l->diffuse.y, l->diffuse.z);
                    ImGui::Text("Specular: %.3f, %.3f, %.3f", l->specular.x, l->specular.y, l->specular.z);
                    ImGui::Text("Intensity: %.3f", l->intensity);
                    ImGui::Text("Cast shadow: %s", l->castShadow ? "true" : "false");
                    ImGui::TreePop();
                }
            }
            for (auto l : pointLights) {
                const std::string& name = l->gameObject->GetName();
                if (ImGui::TreeNode(name.c_str(), "%s (light)", name.c_str())) {
                    ImGui::Text("Attenuation: %.3f, %.3f, %.3f", l->attenuation.x, l->attenuation.y, l->attenuation.z);
                    ImGui::Text("Ambient: %.3f, %.3f, %.3f", l->ambient.x, l->ambient.y, l->ambient.z);
                    ImGui::Text("Diffuse: %.3f, %.3f, %.3f", l->diffuse.x, l->diffuse.y, l->diffuse.z);
                    ImGui::Text("Specular: %.3f, %.3f, %.3f", l->specular.x, l->specular.y, l->specular.z);
                    ImGui::Text("Intensity: %.3f", l->intensity);
                    ImGui::Text("Cast shadow: %s", l->castShadow ? "true" : "false");
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Canvas")) {
            ImGui::Text("Canvas quads: %d", _canvasQuadCount);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Physics Debug")) {
            ImGui::Text("Debug line count: %d", _debugLineCount);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Textures")) {
            for (auto t : uniShadowMaps) {
                if (ImGui::TreeNode(fmt:: format("Directional shadow map #{}", t).c_str())) {
                    ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(64, 64));
                    ImGui::TreePop();
                }
            }
            for (auto t : omniShadowMaps) {
                // FIXME: cubemap textures are not supported yet
                // if (ImGui::TreeNode(fmt:: format("Point shadow map #{}", t).c_str())) {
                //     ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(64, 64));
                //     ImGui::TreePop();
                // }
            }
            ImGui::Separator();
            if (ImGui::TreeNode(fmt::format("Scene Color RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)sceneColorTexture, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("Scene Depth RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)sceneDepthTexture, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("MSAA Resolve RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)msaaResolveTexture, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("GBuffer Position RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)gBuffer.positionRT, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("GBuffer Normal RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)gBuffer.normalRT, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("GBuffer Albedo RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)gBuffer.albedoRT, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("GBuffer Material RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)gBuffer.materialRT, ImVec2(64, 64));
                ImGui::TreePop();
            }
            if (ImGui::TreeNode(fmt::format("GBuffer Depth RT").c_str())) {
                ImGui::Image((ImTextureID)(intptr_t)gBuffer.depthRT, ImVec2(64, 64));
                ImGui::TreePop();
            }
            ImGui::Separator();
            for (auto t : defaultTextures) {
                if (ImGui::TreeNode(fmt:: format("Default Tex #{}", t).c_str())) {
                    ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(64, 64));
                    ImGui::TreePop();
                }
            }
            ImGui::Separator();
            for (auto t : textures) {
                if (ImGui::TreeNode(fmt:: format("Tex #{}", t).c_str())) {
                    ImGui::Image((ImTextureID)(intptr_t)t, ImVec2(64, 64));
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Materials")) {
            for (auto m : materials) {
                if (ImGui::TreeNode("Mat")) {
                    ImGui::Text("Base Map ID: %d", m->baseMap);
                    ImGui::Text("Normal Map ID: %d", m->normalMap);
                    ImGui::Text("AO Map ID: %d", m->aoMap);
                    ImGui::Text("Roughness Map ID: %d", m->roughnessMap);
                    ImGui::Text("Metallic Map ID: %d", m->metallicMap);
                    ImGui::Text("Height Map ID: %d", m->heightMap);
                    ImGui::Text("Ambient: %.3f, %.3f, %.3f", m->ambient.x, m->ambient.y, m->ambient.z);
                    ImGui::Text("Diffuse: %.3f, %.3f, %.3f", m->diffuse.x, m->diffuse.y, m->diffuse.z);
                    ImGui::Text("Specular: %.3f, %.3f, %.3f", m->specular.x, m->specular.y, m->specular.z);
                    ImGui::Text("Shininess: %.3f", m->shininess);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
    }
}

void GraphicsServer::Reset() {
    // TODO: this part is unfinished
    glDeleteTextures(defaultTextures.size(), defaultTextures.data());
    defaultTextures.clear();
    glDeleteTextures(textures.size(), textures.data());
    textures.clear();

    for (auto m : materials) {
        delete m;
    }
    materials.clear();
    _namedMaterials.clear();

    for (auto m : meshes) {
        delete m;
    }
    meshes.clear();
    _namedMeshes.clear();
    _meshInstanceMap.clear();

    defaultCamera = nullptr;
    defaultLight = nullptr;
    cameras.clear();
    directionalLights.clear();
    pointLights.clear();
    renderables.clear();
    _namedMaterials.clear();
    _namedShaders.clear();
}

void GraphicsServer::LoadDefaultTextures() {
    LoadTextures({
        "assets/textures/default_diff.jpg",
        "assets/textures/default_norm.jpg",
        "assets/textures/default_ao.jpg",
        "assets/textures/default_rough.jpg",
        "assets/textures/default_metallic.jpg"
    });
}

void GraphicsServer::LoadTextures(const std::vector<std::string>& paths)
{
    int oldCount = textures.size(), newCount = paths.size();
    textures.resize(oldCount + newCount);

    std::vector<std::shared_ptr<Image>> images(newCount);
    for (int i = 0; i < newCount; i++) {
        auto path = paths[i];
        auto image = &images[i];
        JobSystem::Get()->Execute([path, image](int threadID) {
            *image = AssetManager::loadImage(path);
        });
    }
    JobSystem::Get()->Wait();

    glGenTextures(newCount, &textures[oldCount]);
    for (int i = 0; i < newCount; i++) {
        auto img = images[i];
        if (!img) {
            throw std::runtime_error(fmt::format("Failed to load texture at {}\n", paths[i]));
        }

        glBindTexture(GL_TEXTURE_2D, textures[oldCount + i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // float border[] = {1.f, 1.f, 1.f, 1.f};
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border); // for clamp to border wrapping
        switch (img->channelCount) {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img->width, img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->byteArray.data());
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->byteArray.data());
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->byteArray.data());
            break;
        default:
            throw std::runtime_error(fmt::format("Unknown texture format at {}\n", paths[i]));
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void GraphicsServer::LoadDefaultShaders() {
    LoadShaders({
        {
            "color", {
                .vert = "assets/shaders/tbn.vert",
                .frag = "assets/shaders/pbr.frag"
            },
        },
        {
            "debug_line", {
                .vert = "assets/shaders/debug.vert",
                .frag = "assets/shaders/flat.frag",
            }
        },
        {
            "depth", {
                .vert = "assets/shaders/depth_simple.vert",
                .frag = "assets/shaders/depth_simple.frag"
            },
        },
        {
            "depth_cubemap", {
                .vert = "assets/shaders/depth_cubemap.vert",
                .frag = "assets/shaders/depth_cubemap.frag"
            },
        },
        {
            "hdr", {
                .vert = "assets/shaders/hdr.vert",
                .frag = "assets/shaders/hdr_ca.frag"
            },
        },
        {
            "terrain", {
                .vert = "assets/shaders/terrain.vert",
                .frag = "assets/shaders/terrain.frag",
                .tesc = "assets/shaders/terrain.tesc",
                .tese = "assets/shaders/terrain.tese"
            },
        },
        {
            "canvas", {
                .vert = "assets/shaders/canvas.vert",
                .frag = "assets/shaders/canvas.frag"
            }
        },
        {
            "geometry", {
                .vert = "assets/shaders/geometry.vert",
                .frag = "assets/shaders/geometry.frag"
            }
        },
        {
            "lighting", {
                .vert = "assets/shaders/lighting.vert",
                .frag = "assets/shaders/lighting.frag"
            }
        }
    });
}

void GraphicsServer::LoadShaders(const std::unordered_map<std::string, ShaderProgramProps>& shaderDefs) {
    for (const auto& [uName, props] : shaderDefs) {
        shaders.push_back(std::move(ShaderProgram(props)));
        // TODO: check if entry already exists
        ENGINE_LOG("Shader {} loaded", uName);
        _shaderIDMap[uName] = _nextShaderID++;
    }
}

void GraphicsServer::ReloadShaders()
{
    // TODO: reload shaders
}

void GraphicsServer::LoadMaterials(const std::vector<MaterialProps>& materialDefs) {
    for (const auto& mat : materialDefs) {
        materials.push_back(new Material(mat));
    }
}

void GraphicsServer::CheckFramebufferStatus(const std::string& prefix) {
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_UNDEFINED", prefix));
        case GL_FRAMEBUFFER_UNSUPPORTED:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_UNSUPPORTED", prefix));
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", prefix));
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", prefix));
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE", prefix));
    #ifndef __EMSCRIPTEN__
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER", prefix));
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER", prefix));
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS", prefix));
    #endif
        default:
            throw std::runtime_error(fmt::format("{}: Unknown error code {}", prefix, status));
        }
    }
}

void GraphicsServer::CheckErrors(const std::string& prefix) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        // Reference: https://learnopengl.com/In-Practice/Debugging
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION"; break;
    #ifndef __EMSCRIPTEN__
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW"; break;
    #endif
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        default:
            error = "UNKNOWN"; break;
        }
        Console::Get()->Error(fmt::format("{}: {}\n", prefix, error));
    }
}

void GraphicsServer::CreateFBOs() {
    glGenFramebuffers(1, &shadowFBO);
    glGenFramebuffers(1, &sceneFBO);
    glGenFramebuffers(1, &msaaResolveFBO);
    glGenFramebuffers(1, &gBuffer.id);
}

void GraphicsServer::DestroyFBOs() {
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteFramebuffers(1, &sceneFBO);
    glDeleteFramebuffers(1, &msaaResolveFBO);
    glDeleteFramebuffers(1, &gBuffer.id);
}

void GraphicsServer::CreateRTs(const RenderTargetProps& props) {
    // 1. Create and set shadow pass attachments
    for (int i = 0; i < MAX_UNI_LIGHTS; ++i) {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_2D, map);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        uniShadowMaps[i] = map;
    }
    for (int i = 0; i < MAX_OMNI_LIGHTS; ++i) {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, map);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        for (int f = 0; f < 6; ++f) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        omniShadowMaps[i] = map;
    }

#ifndef __EMSCRIPTEN__
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    for (int i = 0; i < (int)uniShadowMaps.size(); ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uniShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    for (int i = 0; i < (int)omniShadowMaps.size(); ++i) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omniShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
#endif
    CheckErrors("Create shadow RTs");

    // 2. Create and set HDR pass attachments
    glGenTextures(1, &sceneColorTexture);
#ifdef __EMSCRIPTEN__
    glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.width, props.height, 0, GL_RGBA, GL_FLOAT, NULL);
#else
    if (_currRenderPath == RenderPath::Forward) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneColorTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_RGBA16F, props.width, props.height, GL_TRUE);
    } else {
        glBindTexture(GL_TEXTURE_2D, sceneColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.width, props.height, 0, GL_RGBA, GL_FLOAT, NULL);
    }
#endif
    if (glIsTexture(sceneColorTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR color texture");
    }

    glGenTextures(1, &sceneDepthTexture);
#ifdef __EMSCRIPTEN__
    glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, props.width, props.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
#else
    if (_currRenderPath == RenderPath::Forward) {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_DEPTH_COMPONENT, props.width, props.height, GL_TRUE);
    } else {
        glBindTexture(GL_TEXTURE_2D, sceneDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, props.width, props.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
#endif
    if (glIsTexture(sceneDepthTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR depth texture");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
#ifdef __EMSCRIPTEN__
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);
#else
    if (_currRenderPath == RenderPath::Forward) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, sceneColorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture, 0);
    } else {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTexture, 0);
    }
#endif
    CheckFramebufferStatus("HDR framebuffer incomplete");
    CheckErrors("Create HDR RTs");

    // 3. Create and set MSAA resolve pass attachments
    glGenTextures(1, &msaaResolveTexture);
    glBindTexture(GL_TEXTURE_2D, msaaResolveTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.width, props.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, msaaResolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, msaaResolveTexture, 0);
    CheckFramebufferStatus("MSAA framebuffer incomplete");
    CheckErrors("Create MSAA resolve RTs");

    // 4. Create and set geometry pass attachments
    glGenTextures(1, &gBuffer.positionRT);
    glBindTexture(GL_TEXTURE_2D, gBuffer.positionRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, props.width, props.height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &gBuffer.normalRT);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normalRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, props.width, props.height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &gBuffer.albedoRT);
    glBindTexture(GL_TEXTURE_2D, gBuffer.albedoRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, props.width, props.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &gBuffer.materialRT);
    glBindTexture(GL_TEXTURE_2D, gBuffer.materialRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, props.width, props.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &gBuffer.depthRT);
    glBindTexture(GL_TEXTURE_2D, gBuffer.depthRT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, props.width, props.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glGenFramebuffers(1, &gBuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.positionRT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normalRT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.albedoRT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gBuffer.materialRT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depthRT, 0);
    std::array<GLuint,4> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(attachments.size(), attachments.data());
    CheckFramebufferStatus("G-buffer incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckErrors("Create G-buffer RTs");
}

void GraphicsServer::DestroyRTs() {
    glDeleteTextures(1, &sceneColorTexture);
    glDeleteTextures(1, &sceneDepthTexture);
    glDeleteTextures(1, &msaaResolveTexture);

    glDeleteTextures(1, &gBuffer.positionRT);
    glDeleteTextures(1, &gBuffer.normalRT);
    glDeleteTextures(1, &gBuffer.albedoRT);
    glDeleteTextures(1, &gBuffer.materialRT);
    glDeleteTextures(1, &gBuffer.depthRT);
}

void GraphicsServer::CreateCanvasVAO()
{
    glGenVertexArrays(1, &canvasVAO);
    glGenBuffers(1, &canvasVBO);

    glBindVertexArray(canvasVAO);
    glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)offsetof(CanvasVertex, texCoord));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)offsetof(CanvasVertex, color));
    glVertexAttribPointer(3, 1, GL_INT, GL_FALSE, sizeof(CanvasVertex), (void*)offsetof(CanvasVertex, texIndex));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
}

void GraphicsServer::CreateScreenVAO()
{
    std::array<ScreenVertex, 4> verts = {{
        { { -1.0f,  1.0f }, { 0.0f, 1.0f } },
        { { -1.0f, -1.0f }, { 0.0f, 0.0f } },
        { { 1.0f,  1.0f }, { 1.0f, 1.0f } },
        { { 1.0f, -1.0f }, { 1.0f, 0.0f } },
    }};
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);

    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(ScreenVertex), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ScreenVertex), (void*)(offsetof(ScreenVertex, texCoord)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void GraphicsServer::CreateDebugVAO()
{
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);

    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void GraphicsServer::ShadowPass(float dt)
{
    glViewport(0, 0, SHADOW_W, SHADOW_H);
#ifndef __EMSCRIPTEN__
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    auto mainLight = GetMainLight();

    // 1. Render shadow map for directional light
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uniShadowMaps[0], 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    auto depthShader = GetShaderByName("depth");
    depthShader.Activate();
    depthShader.SetUniform(std::string("ProjectionView"), mainLight->GetProjectionMatrix(0) * mainLight->GetViewMatrix());

    for (const auto& [mesh, instances] : _meshInstanceMap)
    {
        if (instances.empty())
            continue;

        if (!mesh->initialized)
            throw std::runtime_error(fmt::format("Mesh uninitialized!"));

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        if (mesh->GetMaterial()->cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        if (mesh->type == MeshType::PRIM) {
            glBindVertexArray(mesh->vao);
            if (instances.size() > 0) { // TODO: use non-instanced rendering for one-off meshes
                glBindBuffer(GL_ARRAY_BUFFER, mesh->ibo);
                glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
                glDrawElementsInstanced(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0, instances.size());
            } else {
                glDrawElements(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0);
            }
            glBindVertexArray(0);
        }
    }

    // 2. Render shadow cubemaps for omni-directional lights
    auto depthCubemapShader = GetShaderByName("depth_cubemap");
    depthCubemapShader.Activate();
    int auxShadows = 0;
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light* l = pointLights[i];
        if (!l->castShadow)
            continue;
        if (auxShadows++ >= MAX_OMNI_LIGHTS)
            break;

        for (int f = 0; f < 6; ++f)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, omniShadowMaps[i], 0);
            glClear(GL_DEPTH_BUFFER_BIT);
            depthCubemapShader.SetUniform(std::string("LightPosition"), l->GetPosition());
            depthCubemapShader.SetUniform(std::string("ProjectionView"), l->GetProjectionMatrix(0) * l->GetViewMatrix(face));

            for (const auto& [mesh, instances] : _meshInstanceMap)
            {
                if (instances.empty())
                    continue;

                if (!mesh->initialized)
                    throw std::runtime_error(fmt::format("Mesh uninitialized!"));

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);

                if (mesh->GetMaterial()->cullFaceEnabled)
                    glEnable(GL_CULL_FACE);
                else
                    glDisable(GL_CULL_FACE);

                if (mesh->type == MeshType::PRIM) {
                    glBindVertexArray(mesh->vao);
                    if (instances.size() > 0) { // TODO: use non-instanced rendering for one-off meshes
                        glBindBuffer(GL_ARRAY_BUFFER, mesh->ibo);
                        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
                        glDrawElementsInstanced(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0, instances.size());
                    } else {
                        glDrawElements(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0);
                    }
                    glBindVertexArray(0);
                }
            }
        }
    }
}

void GraphicsServer::ForwardPass(float dt) {
    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    for (int i = 0; i < MAX_UNI_LIGHTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, uniShadowMaps[i]);
    }
    for (int i = 0; i < MAX_OMNI_LIGHTS; ++i) {
        glActiveTexture(GL_TEXTURE0 + UNI_SHADOW_MAP_COUNT + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, omniShadowMaps[i]);
    }
    for (int i = 0; i < defaultTextures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + DEFAULT_TEXTURE_BASE_INDEX + i);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[i]);
    }
    for (int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + SCENE_TEXTURE_BASE_INDEX + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    auto mainLight = GetMainLight();
    glm::vec3 eyePos = GetMainCamera()->GetEyePosition();
    glm::mat4 projectionView = GetMainCamera()->GetProjectionMatrix() * GetMainCamera()->GetViewMatrix();

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto terrainShader = GetShaderByName("terrain");
    auto colorShader = GetShaderByName("color");
    for (const auto& [mesh, instances] : _meshInstanceMap)
    {
        if (instances.empty())
            continue;

        if (!mesh->initialized)
            throw std::runtime_error(fmt::format("Mesh uninitialized!"));

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // glEnable(GL_STENCIL_TEST);
        // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        // glStencilFunc(GL_ALWAYS, 1, 0xFF);

        // Outline rendering
        // glStencilMask(0xFF);
        // glStencilFunc(GL_ALWAYS, 1, 0xFF);
        // /*
        // pass 1
        // ...
        //  */
        // glStencilMask(0x00);
        // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        // glDepthFunc(GL_ALWAYS);
        // /*
        // pass 2 (scaled)
        // ...
        //  */
        // glDepthFunc(GL_LESS);

        // glStencilMask(0xFF);
        // glStencilFunc(GL_ALWAYS, 1, 0xFF);

    #ifndef __EMSCRIPTEN__
        if (wireframeEnabled || mesh->GetMaterial()->polygonMode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    #endif

        if (mesh->GetMaterial()->cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        // glEnable(GL_PRIMITIVE_RESTART);

        switch (mesh->type) {

        case MeshType::TERRAIN:
            terrainShader.Activate();
            terrainShader.SetUniform(std::string("cam_pos"), eyePos);
            terrainShader.SetUniform(std::string("main_light.direction"), mainLight->direction);
            terrainShader.SetUniform(std::string("main_light.ambient"), mainLight->ambient);
            terrainShader.SetUniform(std::string("main_light.diffuse"), mainLight->diffuse);
            terrainShader.SetUniform(std::string("main_light.specular"), mainLight->specular);
            terrainShader.SetUniform(std::string("main_light.intensity"), mainLight->intensity);
            terrainShader.SetUniform(std::string("main_light.cast_shadow"), mainLight->castShadow ? 1 : 0);
            terrainShader.SetUniform(std::string("main_light.ProjectionView"), mainLight->GetProjectionViewMatrix(0));

            terrainShader.SetUniform(std::string("surf_params.diffuse"), mesh->GetMaterial()->diffuse);
            terrainShader.SetUniform(std::string("surf_params.specular"), mesh->GetMaterial()->specular);
            terrainShader.SetUniform(std::string("surf_params.ambient"), mesh->GetMaterial()->ambient);
            terrainShader.SetUniform(std::string("surf_params.shininess"), mesh->GetMaterial()->shininess);

            terrainShader.SetUniform(std::string("tessellation_factor"), (float)16.0);
            terrainShader.SetUniform(std::string("height_scale"), (float)32.0);
            terrainShader.SetUniform(std::string("height_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->heightMap);
            terrainShader.SetUniform(std::string("ProjectionView"), projectionView);
            terrainShader.SetUniform(std::string("World"), instances[0].modelMatrix);

            glBindVertexArray(mesh->vao);
#ifndef __EMSCRIPTEN__
            glDrawArrays(GL_PATCHES, 0, mesh->vertCount);
#else
            glDrawArrays(GL_TRIANGLES, 0, mesh->vertCount);
#endif
            glBindVertexArray(0);
            break;

        case MeshType::SKY:
            // TODO: implement skybox rendering
            break;

        case MeshType::PRIM:
        default:
            colorShader.Activate();
            colorShader.SetUniform(std::string("cam_pos"), eyePos);
            colorShader.SetUniform(std::string("time"), 0);
            colorShader.SetUniform(std::string("main_light.direction"), mainLight->direction);
            colorShader.SetUniform(std::string("main_light.ambient"), mainLight->ambient);
            colorShader.SetUniform(std::string("main_light.diffuse"), mainLight->diffuse);
            colorShader.SetUniform(std::string("main_light.specular"), mainLight->specular);
            colorShader.SetUniform(std::string("main_light.intensity"), mainLight->intensity);
            colorShader.SetUniform(std::string("main_light.cast_shadow"), mainLight->castShadow ? 1 : 0);
            colorShader.SetUniform(std::string("main_light.ProjectionView"), mainLight->GetProjectionViewMatrix(0));
            for (int i = 0; i < auxLightCount; ++i)
            {
                Light* l = pointLights[i];
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].position"), l->GetPosition());
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ambient"), l->ambient);
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].diffuse"), l->diffuse);
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].specular"), l->specular);
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].attenuation"), l->attenuation);
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].intensity"), l->intensity);
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].cast_shadow"), l->castShadow ? 1 : 0);
                for (int f = 0; f < 6; ++f)
                {
                    GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
                    colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ProjectionViews[") + std::to_string(f) + std::string("]"), l->GetProjectionViewMatrix(0, face));
                }
            }
            colorShader.SetUniform(std::string("aux_light_count"), auxLightCount);
            colorShader.SetUniform(std::string("shadow_map_unit"), (int)0);
            colorShader.SetUniform(std::string("omni_shadow_map_unit"), (int)UNI_SHADOW_MAP_COUNT);
            colorShader.SetUniform(std::string("ProjectionView"), projectionView);
            // Surface parameters
            colorShader.SetUniform(std::string("surf_params.diffuse"), mesh->GetMaterial()->diffuse);
            colorShader.SetUniform(std::string("surf_params.specular"), mesh->GetMaterial()->specular);
            colorShader.SetUniform(std::string("surf_params.ambient"), mesh->GetMaterial()->ambient);
            colorShader.SetUniform(std::string("surf_params.shininess"), mesh->GetMaterial()->shininess);

            // Material textures
            if (mesh->GetMaterial()->baseMap >= 0) {
                colorShader.SetUniform(std::string("base_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->baseMap);
            } else {
                colorShader.SetUniform(std::string("base_map_unit"), DEFAULT_TEXTURE_BASE_INDEX + 0);
            }
            if (mesh->GetMaterial()->normalMap >= 0) {
                colorShader.SetUniform(std::string("normal_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->normalMap);
            } else {
                colorShader.SetUniform(std::string("normal_map_unit"), DEFAULT_TEXTURE_BASE_INDEX + 1);
            }
            if (mesh->GetMaterial()->aoMap >= 0) {
                colorShader.SetUniform(std::string("ao_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->aoMap);
            } else {
                colorShader.SetUniform(std::string("ao_map_unit"), DEFAULT_TEXTURE_BASE_INDEX + 2);
            }
            if (mesh->GetMaterial()->roughnessMap >= 0) {
                colorShader.SetUniform(std::string("roughness_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->roughnessMap);
            } else {
                colorShader.SetUniform(std::string("roughness_map_unit"), DEFAULT_TEXTURE_BASE_INDEX + 3);
            }
            if (mesh->GetMaterial()->metallicMap >= 0) {
                colorShader.SetUniform(std::string("metallic_map_unit"), SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->metallicMap);
            } else {
                colorShader.SetUniform(std::string("metallic_map_unit"), DEFAULT_TEXTURE_BASE_INDEX + 4);
            }

            glBindVertexArray(mesh->vao);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->ibo);
            if (instances.size() > 0) { // TODO: use non-instanced rendering for one-off meshes
                glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
                glDrawElementsInstanced(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0, instances.size());
            } else {
                glDrawElements(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0);
            }
            glBindVertexArray(0);

            break;
        }
    }

    _debugLineCount = debugLines.size() / 2;
    if (debugLines.size() > 0) {
        //glDisable(GL_DEPTH_TEST);
        auto debugShader = GetShaderByName("debug_line");
        debugShader.Activate();
        debugShader.SetUniform(std::string("ProjectionView"), projectionView);

        glBindVertexArray(debugVAO);
        glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
        glBufferData(GL_ARRAY_BUFFER, debugLines.size() * sizeof(DebugVertex), debugLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, debugLines.size());
        glBindVertexArray(0);

        //glEnable(GL_DEPTH_TEST);

        debugLines.clear();
    }
}

void GraphicsServer::GeometryPass(float dt) {
    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);
    std::array<GLuint,4> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(attachments.size(), attachments.data());
    // for (int i = 0; i < MAX_UNI_LIGHTS; ++i) {
    //     glActiveTexture(GL_TEXTURE0 + i);
    //     glBindTexture(GL_TEXTURE_2D, uniShadowMaps[i]);
    // }
    // for (int i = 0; i < MAX_OMNI_LIGHTS; ++i) {
    //     glActiveTexture(GL_TEXTURE0 + UNI_SHADOW_MAP_COUNT + i);
    //     glBindTexture(GL_TEXTURE_CUBE_MAP, omniShadowMaps[i]);
    // }
    for (int i = 0; i < defaultTextures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + DEFAULT_TEXTURE_BASE_INDEX + i);
        glBindTexture(GL_TEXTURE_2D, defaultTextures[i]);
    }
    for (int i = 0; i < textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + SCENE_TEXTURE_BASE_INDEX + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto geometryShader = GetShaderByName("geometry");
    geometryShader.Activate();

    for (const auto& [mesh, instances] : _meshInstanceMap) {
        if (instances.empty())
            continue;

        if (!mesh->initialized)
            throw std::runtime_error("Mesh uninitialized!");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        if (mesh->GetMaterial()->cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        geometryShader.SetUniform("ProjectionView", GetMainCamera()->GetProjectionMatrix() * GetMainCamera()->GetViewMatrix());

        switch (mesh->type) {
        case MeshType::TERRAIN:
            // TODO: implement terrain rendering
            break;
        case MeshType::SKY:
            // TODO: implement skybox rendering
            break;
        case MeshType::PRIM:
        default:
            if (mesh->GetMaterial()->baseMap >= 0) {
                geometryShader.SetUniform("baseMap", SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->baseMap);
            } else {
                geometryShader.SetUniform("baseMap", DEFAULT_TEXTURE_BASE_INDEX + 0);
            }
            if (mesh->GetMaterial()->normalMap >= 0) {
                geometryShader.SetUniform("normalMap", SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->normalMap);
            } else {
                geometryShader.SetUniform("normalMap", DEFAULT_TEXTURE_BASE_INDEX + 1);
            }
            if (mesh->GetMaterial()->aoMap >= 0) {
                geometryShader.SetUniform("aoMap", SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->aoMap);
            } else {
                geometryShader.SetUniform("aoMap", DEFAULT_TEXTURE_BASE_INDEX + 2);
            }
            if (mesh->GetMaterial()->roughnessMap >= 0) {
                geometryShader.SetUniform("roughnessMap", SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->roughnessMap);
            } else {
                geometryShader.SetUniform("roughnessMap", DEFAULT_TEXTURE_BASE_INDEX + 3);
            }
            if (mesh->GetMaterial()->metallicMap >= 0) {
                geometryShader.SetUniform("metallicMap", SCENE_TEXTURE_BASE_INDEX + mesh->GetMaterial()->metallicMap);
            } else {
                geometryShader.SetUniform("metallicMap", DEFAULT_TEXTURE_BASE_INDEX + 4);
            }

            glBindVertexArray(mesh->vao);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->ibo);
            if (instances.size() > 0) {
                glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
                glDrawElementsInstanced(mesh->GetMaterial()->primitiveType, mesh->triCount * 3, GL_UNSIGNED_SHORT, 0, instances.size());
            }
            glBindVertexArray(0);
        }
    }

    CheckErrors("Geometry pass");
}

void GraphicsServer::LightingPass(float dt) {
    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto lightingShader = GetShaderByName("lighting");
    lightingShader.Activate();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.positionRT);
    lightingShader.SetUniform("gPosition", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normalRT);
    lightingShader.SetUniform("gNormal", 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.albedoRT);
    lightingShader.SetUniform("gAlbedo", 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gBuffer.materialRT);
    lightingShader.SetUniform("gMaterial", 3);

    auto mainLight = GetMainLight();
    lightingShader.SetUniform("cam_pos", GetMainCamera()->GetEyePosition());
    lightingShader.SetUniform("mainLight.direction", mainLight->direction);
    lightingShader.SetUniform("mainLight.ambient", mainLight->ambient);
    lightingShader.SetUniform("mainLight.diffuse", mainLight->diffuse);
    lightingShader.SetUniform("mainLight.specular", mainLight->specular);
    lightingShader.SetUniform("mainLight.intensity", mainLight->intensity);

    for (int i = 0; i < auxLightCount; ++i) {
        Light* l = pointLights[i];
        std::string prefix = "pointLights[" + std::to_string(i) + "]";
        lightingShader.SetUniform(prefix + ".position", l->GetPosition());
        lightingShader.SetUniform(prefix + ".ambient", l->ambient);
        lightingShader.SetUniform(prefix + ".diffuse", l->diffuse);
        lightingShader.SetUniform(prefix + ".specular", l->specular);
        lightingShader.SetUniform(prefix + ".attenuation", l->attenuation);
        lightingShader.SetUniform(prefix + ".intensity", l->intensity);
    }
    lightingShader.SetUniform("pointLightCount", auxLightCount);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    CheckErrors("Lighting pass");
}

void GraphicsServer::MSAAResolvePass(float dt)
{
#ifndef __EMSCRIPTEN__
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    auto [width, height] = Window::Get()->GetFramebufferSize();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessEnabled ? msaaResolveFBO : finalFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}


void GraphicsServer::CanvasPass(float dt)
{
    _canvasQuadCount = canvasDrawList.size() / 6;
    if (canvasDrawList.size() > 0) {

        auto [width, height] = Window::Get()->GetFramebufferSize();

        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, postProcessEnabled ? msaaResolveFBO : finalFBO);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef __EMSCRIPTEN__
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

        // TODO: use canvas textures
        for (int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }

        auto canvasShader = GetShaderByName("canvas");
        canvasShader.Activate();
        glm::mat4 projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
        canvasShader.SetUniform(std::string("Projection"), projection);
        for (int i = 0; i < MAX_CANVAS_TEXTURES; i++) {
            canvasShader.SetUniform(fmt::format("Textures[{}]", i), i);
        }

        glBindVertexArray(canvasVAO);
        glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
        glBufferData(GL_ARRAY_BUFFER, canvasDrawList.size() * sizeof(CanvasVertex), canvasDrawList.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, canvasDrawList.size());
        glBindVertexArray(0);
        canvasDrawList.clear();
    }
}

void GraphicsServer::PostProcessPass(float dt)
{
    auto size = Window::Get()->GetFramebufferSize();

    glViewport(0, 0, size.width, size.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glDisable(GL_DEPTH_TEST);
    // glDisable(GL_BLEND);
#ifndef __EMSCRIPTEN__
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, msaaResolveTexture);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto postProcessShader = GetShaderByName("hdr");
    postProcessShader.Activate();
    postProcessShader.SetUniform(std::string("color_map_unit"), (int)0);
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GraphicsServer::PushDebugLine(DebugVertex from, DebugVertex to)
{
    debugLines.push_back(from);
    debugLines.push_back(to);
}

void GraphicsServer::PushCanvasQuad(
    float x, float y, float w, float h, float angle, float pivotX, float pivotY,
    const glm::vec4& color, int texIndex, const glm::vec2& uvMin, const glm::vec2& uvMax)
{
    glm::vec2 pivotOffset = glm::vec2(w * pivotX, h * pivotY);
    // glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
    // transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    // transform = glm::scale(transform, glm::vec3(size, 1.0f));

    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(x + pivotOffset.x, y + pivotOffset.y, 0.0f));
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(w, h, 1.0f));
    glm::mat4 O = glm::translate(glm::mat4(1.0f), glm::vec3(-pivotX, -pivotY, 0.0f));
    glm::mat4 transform = T * R * S * O;

    glm::vec4 bl = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 br = transform * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 tr = transform * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    glm::vec4 tl = transform * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

    canvasDrawList.push_back({
        glm::vec2(bl),
        glm::vec2(uvMin.x, uvMin.y),
        color,
        texIndex
    });
    canvasDrawList.push_back({
        glm::vec2(br),
        glm::vec2(uvMax.x, uvMin.y),
        color,
        texIndex
    });
    canvasDrawList.push_back({
        glm::vec2(tr),
        glm::vec2(uvMax.x, uvMax.y),
        color,
        texIndex
    });
    canvasDrawList.push_back({
        glm::vec2(bl),
        glm::vec2(uvMin.x, uvMin.y),
        color,
        texIndex
    });
    canvasDrawList.push_back({
        glm::vec2(tr),
        glm::vec2(uvMax.x, uvMax.y),
        color,
        texIndex
    });
    canvasDrawList.push_back({
        glm::vec2(tl),
        glm::vec2(uvMin.x, uvMax.y),
        color,
        texIndex
    });
}

void GraphicsServer::PushCanvasQuadTiled(
    float x, float y, float w, float h, float angle, float pivotX, float pivotY,
    const glm::vec4& color, int texIndex, const glm::vec2& tilesetSize, const glm::vec2& tileIndex)
{
    glm::vec2 uvMin = tileIndex / tilesetSize;
    glm::vec2 uvMax = (tileIndex + glm::vec2(1.0f)) / tilesetSize;
    PushCanvasQuad(x, y, w, h, angle, pivotX, pivotY, color, texIndex, uvMin, uvMax);
}

Material* GraphicsServer::CreateMaterial(Material* material)
{
    if (material) {
        materials.push_back(material);
        return material;
    } else {
        auto emptyMaterial = new Material({});
        materials.push_back(emptyMaterial);
        return emptyMaterial;
    }
}

Material* GraphicsServer::CreateMaterial(const std::string& name, Material* material)
{
    if (material) {
        materials.push_back(material);
        _namedMaterials.insert({name, material});
        return material;
    } else {
        auto emptyMaterial = new Material({});
        materials.push_back(emptyMaterial);
        _namedMaterials.insert({name, emptyMaterial});
        return emptyMaterial;
    }
}


Mesh* GraphicsServer::CreateMesh(Mesh* mesh)
{
    if (mesh) {
        meshes.push_back(mesh);
        return mesh;
    } else {
        auto emptyMesh = new Mesh();
        meshes.push_back(emptyMesh);
        return emptyMesh;
    }
}

Mesh* GraphicsServer::CreateMesh(const std::string& name, Mesh* mesh)
{
    if (mesh) {
        meshes.push_back(mesh);
        _namedMeshes.insert({name, mesh});
        return mesh;
    } else {
        auto emptyMesh = new Mesh();
        meshes.push_back(emptyMesh);
        _namedMeshes.insert({name, emptyMesh});
        return emptyMesh;
    }
}

Mesh* GraphicsServer::CreateCubeMesh(const std::string& name, float size)
{
    auto mesh = MeshBuilder::CreateCube(size);
    meshes.push_back(mesh);
    _namedMeshes.insert({name, mesh});
    return mesh;
}

Mesh* GraphicsServer::CreateSphereMesh(const std::string& name, float radius, int division)
{
    auto mesh = MeshBuilder::CreateSphere(radius, division);
    meshes.push_back(mesh);
    _namedMeshes.insert({name, mesh});
    return mesh;
}

Mesh* GraphicsServer::CreateCapsuleMesh(const std::string& name, float radius, float height)
{
    // TODO: unimplemented
    // auto mesh = MeshBuilder::CreateCapsule(radius, height);
    // meshes.push_back(mesh);
    // _namedMeshes.insert({name, mesh});
    // return mesh;
    return new Mesh;
}

Mesh* GraphicsServer::CreateTerrainMesh(const std::string& name, float worldSize, int resolution)
{
    auto mesh = MeshBuilder::CreateTerrain(worldSize, resolution);
    meshes.push_back(mesh);
    _namedMeshes.insert({name, mesh});
    return mesh;
}

Renderable* GraphicsServer::CreateRenderable(GameObject* go, Mesh* mesh)
{
    auto renderable = new Renderable(go, mesh);
    renderables.push_back(renderable);
    return renderable;
}

Drawable2D* GraphicsServer::CreateDrawable2D(GameObject* go, const Drawable2DProps& props)
{
    auto drawable = new Drawable2D(go, props);
    canvasDrawables.push_back(drawable);
    return drawable;
}

Camera* GraphicsServer::CreateCamera(GameObject* go, const CameraProps& props)
{
    auto camera = new Camera(go, props);
    cameras.push_back(camera);
    return camera;
}

Light* GraphicsServer::CreateLight(GameObject* go, const LightProps& props)
{
    auto light = new Light(go, props);
    if (props.type == LightType::Point) {
        pointLights.push_back(light);
    } else if (props.type == LightType::Directional) {
        directionalLights.push_back(light);
    }
    return light;
}