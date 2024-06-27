#include "graphics_server.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "game_object.hpp"
#include "application.hpp"

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

GraphicsServer::GraphicsServer()
{

}

GraphicsServer::~GraphicsServer()
{
    for (auto& mat : materials)
        delete mat;
    glDeleteTextures(textures.size(), textures.data());
}

void GraphicsServer::Init(MessageBus* mb, Application* app)
{
    Server::Init(mb, app);

    stbi_set_flip_vertically_on_load(true);

    // Note that OpenGL extensions must NOT be initialzed before the window creation
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize glew!");

    glPrimitiveRestartIndex(0xFFFF);
    glCullFace(GL_BACK);
    #if MSAA_ON
    glEnable(GL_MULTISAMPLE);
    #endif

    auto window = this->_app->GetWindow();

    auto size = window->GetSize();
    this->CreateRenderTargets(RenderTargetProps(size.x, size.y));
    this->CreateScreenVAO();

    window->SetOnResize([this, window](int width, int height) {
        if (window->IsClosing())
            return;
        this->UpdateRenderTargets(RenderTargetProps(width, height));
    });


}

void GraphicsServer::Process(float dt)
{

}

void GraphicsServer::Render(float dt)
{
    // TODO: Put the logic of generating command buffers here
    // Setup
    meshInstances.clear();
    for (auto rend : renderables)
    {
        Mesh* model = rend->mesh;
        glm::mat4 wm = rend->gameObject->GetTransform();
        meshInstances[model].push_back(wm);
    }
    auxLightCount = (int)lights.size() - mainLightCount;

    ShadowPass(dt);
    ColorPass(dt);
    MSAAPass(dt);
    PostProcessPass(dt);
}

void GraphicsServer::RenderUI(float dt)
{
    ImGui::Begin("General Graphics");

    ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
    ImGui::Text("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
    ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));

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

    ImGui::End();

    ImGui::Begin("Realtime Rendering");

    ImGui::ColorEdit3("Clear color", (float*)&clearColor);
    //ImGui::Text("Entities count: %lu", entities.size());
    ImGui::Text("Draw time: %.3f s/frame", dt);
    ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
    //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();
}

void GraphicsServer::OnMessage(Message msg)
{

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

        int width, height, nChannels;
        unsigned char* data = stbi_load(paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            throw std::runtime_error(fmt::format("Failed to load texture at {}\n", paths[i]));
        }
        stbi_image_free(data);
    }
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

void GraphicsServer::CreateScreenVAO()
{
    std::vector<GLfloat> verts = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);

    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void GraphicsServer::ShadowPass(float dt)
{
    glViewport(0, 0, SHADOW_W, SHADOW_H);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GetShadowMap(DIR_LIGHT, 0), 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthTextureProgram.Activate();
    depthTextureProgram.SetUniform(std::string("ProjectionView"), lights[0]->GetProjectionMatrix(0) * lights[0]->GetViewMatrix());

    for (const auto& [name, mesh] : Mesh::MeshList)
    {
        if (meshInstances.count(mesh) == 0)
            continue;
        mesh->Render(depthTextureProgram, meshInstances.find(mesh)->second);
    }

    int auxShadows = 0;
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light* l = lights[i + mainLightCount];
        if ((bool)l->castShadow)
            continue;
        if (auxShadows++ >= MAX_OMNI_LIGHTS)
            break;

        for (int f = 0; f < 6; ++f)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, GetShadowMap(POINT_LIGHT, i), 0);
            glClear(GL_DEPTH_BUFFER_BIT);
            depthCubemapProgram.Activate();
            depthCubemapProgram.SetUniform(std::string("LightPosition"), l->position);
            depthCubemapProgram.SetUniform(std::string("ProjectionView"), l->GetProjectionMatrix(0) * l->GetViewMatrix(face));

            for (const auto& [name, model] : Mesh::MeshList)
            {
                if (meshInstances.count(model) == 0)
                    continue;
                model->Render(depthCubemapProgram, meshInstances.find(model)->second);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsServer::ColorPass(float dt)
{
    auto size = this->_app->GetWindow()->GetSize();

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
    colorProgram.Activate();
    glm::mat4 cameraTransform = cameras[0]->gameObject->GetModelWorldTransform();
    glm::vec3 eyePos = cameras[0]->GetEye(cameraTransform);
    colorProgram.SetUniform(std::string("cam_pos"), eyePos);
    colorProgram.SetUniform(std::string("time"), 0);
    colorProgram.SetUniform(std::string("main_light.direction"), lights[0]->direction);
    colorProgram.SetUniform(std::string("main_light.ambient"), lights[0]->ambient);
    colorProgram.SetUniform(std::string("main_light.diffuse"), lights[0]->diffuse);
    colorProgram.SetUniform(std::string("main_light.specular"), lights[0]->specular);
    colorProgram.SetUniform(std::string("main_light.intensity"), lights[0]->intensity);
    colorProgram.SetUniform(std::string("main_light.cast_shadow"), lights[0]->castShadow);
    colorProgram.SetUniform(std::string("main_light.ProjectionView"), lights[0]->GetProjectionViewMatrix(0));
    for (int i = 0; i < auxLightCount; ++i)
    {
        Light* l = lights[i + mainLightCount];
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].position"), l->position);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ambient"), l->ambient);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].diffuse"), l->diffuse);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].specular"), l->specular);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].attenuation"), l->attenuation);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].intensity"), l->intensity);
        colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].cast_shadow"), l->castShadow);
        for (int f = 0; f < 6; ++f)
        {
            GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + f;
            colorProgram.SetUniform(std::string("aux_lights[") + std::to_string(i) + std::string("].ProjectionViews[") + std::to_string(f) + std::string("]"), l->GetProjectionViewMatrix(0, face));
        }
    }
    colorProgram.SetUniform(std::string("aux_light_count"), auxLightCount);
    colorProgram.SetUniform(std::string("shadow_map_unit"), (int)0);
    colorProgram.SetUniform(std::string("omni_shadow_map_unit"), (int)1);
    colorProgram.SetUniform(std::string("ProjectionView"), cameras[0]->GetProjectionMatrix() * cameras[0]->GetViewMatrix(cameraTransform));

    for (const auto& [name, model] : Mesh::MeshList)
    {
        if (meshInstances.count(model) == 0)
            continue;
        model->Render(colorProgram, meshInstances.find(model)->second);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsServer::MSAAPass(float dt)
{
    auto size = this->_app->GetWindow()->GetSize();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, hdrFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msaaFBO);
    glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GraphicsServer::PostProcessPass(float dt)
{
    auto size = this->_app->GetWindow()->GetSize();

    glViewport(0, 0, size.x, size.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    hdrProgram.Activate();
    hdrProgram.SetUniform(std::string("color_map_unit"), (int)0);
    // hdrProgram.SetUniform(std::string("exposure"), (float)1.0);
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}