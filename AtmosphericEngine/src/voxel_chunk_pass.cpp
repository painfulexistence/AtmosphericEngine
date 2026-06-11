#include "renderer.hpp"
#include "asset_manager.hpp"
#include "camera_component.hpp"
#include "graphics_server.hpp"
#include "light_component.hpp"
#include "mesh.hpp"
#include "gl_buffer.hpp"
#include "gl_render_target.hpp"
#include "window.hpp"

#include <glm/gtc/matrix_transform.hpp>

// ---- Helper: build/draw the full-screen quad ---------------------------------
static void DrawScreenQuadVAO(GLuint vao) {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ============================================================================
//  SkyboxPass  (gradient sky, rendered at depth = 1)
// ============================================================================
void SkyboxPass::Execute(GraphicsServer* ctx, Renderer& renderer, CommandEncoder* /*enc*/) {
    if (renderer.skyboxVAO == 0) return;

    ShaderProgram* shader = AssetManager::Get().GetShader("skybox");
    if (!shader) return;

    CameraComponent* camera = ctx->GetMainCamera();
    if (!camera) return;

    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);

    GLuint targetFBO = renderer.postProcessEnabled
        ? static_cast<GLRenderTarget*>(renderer.sceneRT.get())->GetNativeFBOID()
        : renderer.finalFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    shader->Activate();

    // Strip translation from view matrix so sky doesn't move with camera
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(camera->GetViewMatrix()));
    shader->SetUniform("u_proj",         camera->GetProjectionMatrix());
    shader->SetUniform("u_view",         viewNoTranslation);
    shader->SetUniform("u_skyColor",     skyColor);
    shader->SetUniform("u_horizonColor", horizonColor);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(renderer.skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    shader->Deactivate();
}

// ============================================================================
//  VoxelChunkPass
// ============================================================================
void VoxelChunkPass::Execute(GraphicsServer* ctx, Renderer& renderer, CommandEncoder* /*enc*/) {
    const auto& queue = renderer.GetOpaqueQueue();
    if (queue.empty()) return;

    ShaderProgram* shader = AssetManager::Get().GetShader("voxel");
    if (!shader) return;

    CameraComponent* camera = ctx->GetMainCamera();
    if (!camera) return;
    LightComponent*  light  = ctx->GetMainLight();

    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);

    // Render into the same target as ForwardOpaquePass (no clear — already done)
    GLuint targetFBO = renderer.postProcessEnabled
        ? static_cast<GLRenderTarget*>(renderer.sceneRT.get())->GetNativeFBOID()
        : renderer.finalFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);

    shader->Activate();

    glm::mat4 viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
    shader->SetUniform("u_viewProj",    viewProj);
    shader->SetUniform("u_cameraPos",   camera->GetEyePosition());

    glm::vec3 lightDir   = light ? glm::normalize(-light->direction) : glm::vec3(0.5f, 1.0f, 0.3f);
    glm::vec3 lightColor = light ? light->diffuse  : glm::vec3(1.0f);
    glm::vec3 ambient    = light ? light->ambient * 0.15f : glm::vec3(0.1f);
    shader->SetUniform("u_lightDir",    lightDir);
    shader->SetUniform("u_lightColor",  lightColor);
    shader->SetUniform("u_ambientColor",ambient);
    shader->SetUniform("u_fogColor",    glm::vec3(0.55f, 0.65f, 0.75f));
    shader->SetUniform("u_fogDensity",  0.003f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for (const auto& sortable : queue) {
        const auto& cmd = sortable.cmd;
        Mesh* mesh = cmd.mesh;

        if (!mesh || mesh->type != MeshType::VOXEL) continue;
        if (!mesh->UsesRenderMesh()) continue;

        shader->SetUniform("u_model", cmd.transform);

        Buffer* buf = ctx->GetRenderMesh(mesh->GetRenderMeshHandle());
        if (buf && buf->IsInitialized() && buf->GetVertexCount() > 0) {
            buf->Draw(nullptr, PrimitiveTopology::Triangles);
        }
    }

    shader->Deactivate();
}

// ============================================================================
//  WaterPass
// ============================================================================
void WaterPass::Execute(GraphicsServer* ctx, Renderer& renderer, CommandEncoder* /*enc*/) {
    const auto& queue = renderer.GetTransparentQueue();
    if (queue.empty()) return;

    ShaderProgram* shader = AssetManager::Get().GetShader("water");
    if (!shader) return;

    CameraComponent* camera = ctx->GetMainCamera();
    if (!camera) return;
    LightComponent*  light  = ctx->GetMainLight();

    auto [width, height] = Window::Get()->GetFramebufferSize();
    glViewport(0, 0, width, height);

    shader->Activate();

    glm::mat4 viewProj = camera->GetProjectionMatrix() * camera->GetViewMatrix();
    shader->SetUniform("u_viewProj",   viewProj);
    shader->SetUniform("u_cameraPos",  camera->GetEyePosition());
    shader->SetUniform("u_time",       renderer.frameTime);
    shader->SetUniform("u_fogColor",   glm::vec3(0.55f, 0.65f, 0.75f));
    shader->SetUniform("u_fogDensity", 0.003f);

    glm::vec3 lightDir   = light ? glm::normalize(-light->direction) : glm::vec3(0.5f, 1.0f, 0.3f);
    glm::vec3 lightColor = light ? light->diffuse  : glm::vec3(1.0f);
    shader->SetUniform("u_lightDir",   lightDir);
    shader->SetUniform("u_lightColor", lightColor);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (const auto& s : queue) {
        const auto& cmd = s.cmd;
        Mesh* mesh = cmd.mesh;
        if (!mesh) continue;

        Material* mat = mesh->GetMaterial();
        if (!mat || mat->renderQueue != RenderQueue::Transparent) continue;

        shader->SetUniform("u_model", cmd.transform);

        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(mesh->triCount * 3),
                       GL_UNSIGNED_SHORT, nullptr);
        glBindVertexArray(0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    shader->Deactivate();
}

// ============================================================================
//  BloomPass  (pyramid downsample + upsample + ACES composite)
// ============================================================================
void BloomPass::InitMips(int w, int h) {
    for (int i = 0; i < MIP_LEVELS; ++i) {
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        auto& m = _mips[i];
        m.w = w; m.h = h;

        if (m.tex) glDeleteTextures(1, &m.tex);
        if (m.fbo) glDeleteFramebuffers(1, &m.fbo);

        glGenTextures(1, &m.tex);
        glBindTexture(GL_TEXTURE_2D, m.tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &m.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m.tex, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    _initialized = true;
}

void BloomPass::Execute(GraphicsServer* ctx, Renderer& renderer, CommandEncoder* /*enc*/) {
    if (!renderer.postProcessEnabled) return;
    if (!renderer.msaaResolveRT)      return;
    if (renderer.screenQuadVAO == 0)  return;

    auto [w, h] = Window::Get()->GetFramebufferSize();
    if (!_initialized || w != _lastW || h != _lastH) {
        InitMips(w, h);
        _lastW = w; _lastH = h;
    }

    GLuint sceneTexture = static_cast<GLuint>(renderer.msaaResolveRT->GetTextureID());

    ShaderProgram* threshShader  = AssetManager::Get().GetShader("bloom_threshold");
    ShaderProgram* downShader    = AssetManager::Get().GetShader("bloom_downsample");
    ShaderProgram* upShader      = AssetManager::Get().GetShader("bloom_upsample");
    ShaderProgram* compShader    = AssetManager::Get().GetShader("bloom_composite");
    if (!threshShader || !downShader || !upShader || !compShader) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // 1. Threshold pass → mip[0]
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _mips[0].fbo);
        glViewport(0, 0, _mips[0].w, _mips[0].h);
        threshShader->Activate();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sceneTexture);
        threshShader->SetUniform("u_scene",     0);
        threshShader->SetUniform("u_threshold", threshold);
        DrawScreenQuadVAO(renderer.screenQuadVAO);
        threshShader->Deactivate();
    }

    // 2. Downsample chain: mip[i-1] → mip[i]
    downShader->Activate();
    for (int i = 1; i < MIP_LEVELS; ++i) {
        auto& src = _mips[i - 1];
        auto& dst = _mips[i];
        glBindFramebuffer(GL_FRAMEBUFFER, dst.fbo);
        glViewport(0, 0, dst.w, dst.h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, src.tex);
        downShader->SetUniform("u_src",       0);
        downShader->SetUniform("u_texelSize",  glm::vec2(1.0f / src.w, 1.0f / src.h));
        DrawScreenQuadVAO(renderer.screenQuadVAO);
    }
    downShader->Deactivate();

    // 3. Upsample chain: mip[i] → mip[i-1]  (additive blend)
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    upShader->Activate();
    for (int i = MIP_LEVELS - 1; i >= 1; --i) {
        auto& src = _mips[i];
        auto& dst = _mips[i - 1];
        glBindFramebuffer(GL_FRAMEBUFFER, dst.fbo);
        glViewport(0, 0, dst.w, dst.h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, src.tex);
        upShader->SetUniform("u_src",          0);
        upShader->SetUniform("u_texelSize",     glm::vec2(1.0f / src.w, 1.0f / src.h));
        upShader->SetUniform("u_filterRadius",  1.0f);
        DrawScreenQuadVAO(renderer.screenQuadVAO);
    }
    upShader->Deactivate();
    glDisable(GL_BLEND);

    // 4. Composite: scene + bloom → back into msaaResolveRT (linear HDR, no tonemapping)
    // PostProcessPass reads msaaResolveRT and applies HDR tonemapping as the final step.
    GLuint resolveTargetFBO = static_cast<GLRenderTarget*>(renderer.msaaResolveRT.get())->GetNativeFBOID();
    glBindFramebuffer(GL_FRAMEBUFFER, resolveTargetFBO);
    glViewport(0, 0, w, h);
    compShader->Activate();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _mips[0].tex);
    compShader->SetUniform("u_scene",        0);
    compShader->SetUniform("u_bloom",        1);
    compShader->SetUniform("u_bloomStrength", bloomStrength);
    DrawScreenQuadVAO(renderer.screenQuadVAO);
    compShader->Deactivate();

    glEnable(GL_DEPTH_TEST);
}
