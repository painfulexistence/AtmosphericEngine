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

#include <cstddef>

void CheckFramebufferStatus(const char* errorPrefix) {
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_UNDEFINED", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER", errorPrefix));
        case GL_FRAMEBUFFER_UNSUPPORTED:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_UNSUPPORTED", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE", errorPrefix));
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            throw std::runtime_error(fmt::format("{}: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS", errorPrefix));
        default:
            throw std::runtime_error(fmt::format("{}: Unknown error code {}", errorPrefix, status));
        }
    }
}

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

    // Note that OpenGL extensions must NOT be initialzed before the window creation
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize glew!");

    glPrimitiveRestartIndex(0xFFFF);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    glLineWidth(2.0f);

    glCullFace(GL_BACK);
    #if MSAA_ON
    glEnable(GL_MULTISAMPLE);
    #endif

    auto window = Window::Get();
    auto size = window->GetSize();
    CreateRenderTargets(RenderTargetProps(size.x, size.y));

    window->SetOnResize([this, window](int width, int height) {
        if (window->IsClosing())
            return;
        UpdateRenderTargets(RenderTargetProps(width, height));
    });

    CreateDebugVAO();
    CreateScreenVAO();

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
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
        .direction = glm::vec3(0.0f, -1.0f, 0.0f),
        .ambient = glm::vec3(1.0f, 1.0f, 1.0f),
        .diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
        .specular = glm::vec3(1.0f, 1.0f, 1.0f),
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

    auxLightCount = (int)pointLights.size();

    ShadowPass(dt);
    ColorPass(dt);
    DebugPass(dt);
    MSAAPass(dt);
    if (postProcessEnabled)
    {
        PostProcessPass(dt);
    }
}

void GraphicsServer::DrawImGui(float dt)
{
    if (ImGui::CollapsingHeader("Graphics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Clear color", (float*)&clearColor);
        //ImGui::Text("Entities count: %lu", entities.size());
        ImGui::Text("Draw time: %.3f s/frame", dt);
        ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
}

void GraphicsServer::LoadTextures(const std::vector<std::string>& paths)
{
    int count = paths.size();
    textures.resize(count);
    glGenTextures(count, textures.data());

    for (int i = 0; i < count; i++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // float border[] = {1.f, 1.f, 1.f, 1.f};
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border); // for clamp to border wrapping

        auto img = AssetManager::loadImage(paths[i]);
        if (img) {
            int format;
            switch (img->channelCount) {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img->width,img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->byteArray.data());
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
            glGenerateMipmap(GL_TEXTURE_2D); // must be called after glTexImage2D
        } else {
            throw std::runtime_error(fmt::format("Failed to load texture at {}\n", paths[i]));
        }
    }
}

void GraphicsServer::LoadDepthShader(const ShaderProgram& program)
{
    depthShader = program;
}

void GraphicsServer::LoadDepthCubemapShader(const ShaderProgram& program)
{
    depthCubemapShader = program;
}

void GraphicsServer::LoadColorShader(const ShaderProgram& program)
{
    colorShader = program;
}

void GraphicsServer::LoadDebugShader(const ShaderProgram& program)
{
    debugShader = program;
}

void GraphicsServer::LoadTerrainShader(const ShaderProgram& program)
{
    terrainShader = program;
}

void GraphicsServer::LoadPostProcessShader(const ShaderProgram& program)
{
    postProcessShader = program;
}

void GraphicsServer::ReloadShaders()
{
    // TODO: reload shaders
}

void GraphicsServer::CheckErrors()
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode)
        {
            // Reference: https://learnopengl.com/In-Practice/Debugging
            case GL_INVALID_ENUM:
            error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:
            error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:
            error = "UNKNOWN"; break;
        }
        throw std::runtime_error(fmt::format("GL error: {}\n", error));
    }
}

void GraphicsServer::CreateRenderTargets(const RenderTargetProps& props)
{
    // 1. Create framebuffers
    glGenFramebuffers(1, &shadowFBO);
    glGenFramebuffers(1, &hdrFBO);
    glGenFramebuffers(1, &msaaFBO);

    // 2. Create and set shadow pass attachments
    for (int i = 0; i < MAX_UNI_LIGHTS; ++i)
    {
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
    for (int i = 0; i < MAX_OMNI_LIGHTS; ++i)
    {
        GLuint map;
        glGenTextures(1, &map);
        glBindTexture(GL_TEXTURE_CUBE_MAP, map);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        for (int f = 0; f < 6; ++f)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        omniShadowMaps[i] = map;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    for (int i = 0; i < (int)uniShadowMaps.size(); ++i)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uniShadowMaps[0], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    for (int i = 0; i < (int)omniShadowMaps.size(); ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, omniShadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // 3. Create and set HDR pass attachments
    glGenTextures(1, &hdrColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_RGBA16F, props.width, props.height, GL_TRUE);
    if (glIsTexture(hdrColorTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR color texture");
    }

    glGenTextures(1, &hdrDepthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_DEPTH_COMPONENT, props.width, props.height, GL_TRUE);
    if (glIsTexture(hdrDepthTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR depth texture");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture, 0);
    CheckFramebufferStatus("HDR framebuffer incomplete");

    // 4. Create and set MSAA pass attachments
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.width, props.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    CheckFramebufferStatus("MSAA framebuffer incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsServer::UpdateRenderTargets(const RenderTargetProps& props)
{
    // 1. Update and reset HDR pass attachments
    glDeleteTextures(1, &hdrColorTexture);
    glGenTextures(1, &hdrColorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_RGBA16F, props.width, props.height, GL_TRUE);
    if (glIsTexture(hdrColorTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR color texture");
    }

    glDeleteTextures(1, &hdrDepthTexture);
    glGenTextures(1, &hdrDepthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, props.numSamples, GL_DEPTH_COMPONENT, props.width, props.height, GL_TRUE);
    if (glIsTexture(hdrDepthTexture) != GL_TRUE) {
        throw std::runtime_error("Failed to create HDR depth texture");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, hdrColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, hdrDepthTexture, 0);
    CheckFramebufferStatus("HDR framebuffer incomplete");

    // 2. Update and reset MSAA pass attachments
    glDeleteTextures(1, &screenTexture);
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.width, props.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);
    CheckFramebufferStatus("MSAA framebuffer incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)offsetof(CanvasVertex, texId));
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
    auto mainLight = GetMainLight();

    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    // 1. Render shadow map for directional light
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GetShadowMap(LightType::Directional, 0), 0);
    glClear(GL_DEPTH_BUFFER_BIT);
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
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, GetShadowMap(LightType::Point, i), 0);
            glClear(GL_DEPTH_BUFFER_BIT);
            depthCubemapShader.SetUniform(std::string("LightPosition"), l->position);
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

                if (mesh->type == MeshType::TERRAIN) {
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

void GraphicsServer::ColorPass(float dt)
{
    auto size = Window::Get()->GetSize();
    auto mainLight = GetMainLight();

    glViewport(0, 0, size.x, size.y);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, uniShadowMaps[0]);
    for (int i = 0; i < MAX_OMNI_LIGHTS; ++i)
    {
        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, omniShadowMaps[i]);
    }
    for (int i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + NUM_MAP_UNITS + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
    }

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        if (wireframeEnabled || mesh->GetMaterial()->polygonMode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        if (mesh->GetMaterial()->cullFaceEnabled)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        // glEnable(GL_PRIMITIVE_RESTART);

        glm::vec3 eyePos = GetMainCamera()->GetEyePosition();
        glm::mat4 projectionView = GetMainCamera()->GetProjectionMatrix() * GetMainCamera()->GetViewMatrix();
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
            terrainShader.SetUniform(std::string("height_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->heightMap);
            terrainShader.SetUniform(std::string("ProjectionView"), projectionView);
            terrainShader.SetUniform(std::string("World"), instances[0].modelMatrix);

            glBindVertexArray(mesh->vao);
            glDrawArrays(GL_PATCHES, 0, mesh->vertCount);
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
                colorShader.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].position"), l->position);
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
            colorShader.SetUniform(std::string("omni_shadow_map_unit"), (int)1);
            colorShader.SetUniform(std::string("ProjectionView"), projectionView);
            // Surface parameters
            colorShader.SetUniform(std::string("surf_params.diffuse"), mesh->GetMaterial()->diffuse);
            colorShader.SetUniform(std::string("surf_params.specular"), mesh->GetMaterial()->specular);
            colorShader.SetUniform(std::string("surf_params.ambient"), mesh->GetMaterial()->ambient);
            colorShader.SetUniform(std::string("surf_params.shininess"), mesh->GetMaterial()->shininess);

            // Material textures
            if (mesh->GetMaterial()->baseMap >= 0) {
                colorShader.SetUniform(std::string("base_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->baseMap);
            } else {
                colorShader.SetUniform(std::string("base_map_unit"), NUM_MAP_UNITS + 0);
            }
            if (mesh->GetMaterial()->normalMap >= 0) {
                colorShader.SetUniform(std::string("normal_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->normalMap);
            } else {
                colorShader.SetUniform(std::string("normal_map_unit"), NUM_MAP_UNITS + 1);
            }
            if (mesh->GetMaterial()->aoMap >= 0) {
                colorShader.SetUniform(std::string("ao_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->aoMap);
            } else {
                colorShader.SetUniform(std::string("ao_map_unit"), NUM_MAP_UNITS + 2);
            }
            if (mesh->GetMaterial()->roughnessMap >= 0) {
                colorShader.SetUniform(std::string("roughness_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->roughnessMap);
            } else {
                colorShader.SetUniform(std::string("roughness_map_unit"), NUM_MAP_UNITS + 3);
            }
            if (mesh->GetMaterial()->metallicMap >= 0) {
                colorShader.SetUniform(std::string("metallic_map_unit"), NUM_MAP_UNITS + mesh->GetMaterial()->metallicMap);
            } else {
                colorShader.SetUniform(std::string("metallic_map_unit"), NUM_MAP_UNITS + 4);
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
}

void GraphicsServer::DebugPass(float dt)
{
    if (debugLines.size() == 0)
        return;

    glm::vec3 eyePos = GetMainCamera()->GetEyePosition();
    glm::mat4 projectionView = GetMainCamera()->GetProjectionMatrix() * GetMainCamera()->GetViewMatrix();

    //glDisable(GL_DEPTH_TEST);

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

void GraphicsServer::MSAAPass(float dt)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto size = Window::Get()->GetSize();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
    if (postProcessEnabled) {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    } else {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GraphicsServer::PostProcessPass(float dt)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto size = Window::Get()->GetSize();

    glViewport(0, 0, size.x, size.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    postProcessShader.Activate();
    postProcessShader.SetUniform(std::string("color_map_unit"), (int)0);
    // hdrProgram.SetUniform(std::string("exposure"), (float)1.0);
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GraphicsServer::PushDebugLine(DebugVertex from, DebugVertex to)
{
    debugLines.push_back(from);
    debugLines.push_back(to);
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
    // auto mesh = MeshBuilder::CreateCapsule(radius, height);
    // meshes.push_back(mesh);
    // _namedMeshes.insert({name, mesh});
    // return mesh;
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