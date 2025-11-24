#pragma once
#include "asset_manager.hpp"
#include "config.hpp"
#include "glm/mat4x4.hpp"
#include "globals.hpp"
#include "graphics_server.hpp"
#include "mesh.hpp"

class GraphicsServer;
class ShaderProgram;

struct RenderCommand {
    Mesh* mesh;
    ShaderProgram* shader;// Using shader directly for now
    glm::mat4 transform;
    uint32_t sortKey;
};

class RenderPass {
public:
    virtual ~RenderPass() = default;
    virtual void Execute(GraphicsServer* ctx, Renderer& renderer) = 0;
};

class ShadowPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class ForwardOpaquePass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class DeferredGeometryPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class DeferredLightingPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

// For particles, world UI
class TransparentPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class MSAAResolvePass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class CanvasPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class PostProcessPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

// TODO: rename this
class UIPass : public RenderPass {
public:
    void Execute(GraphicsServer* ctx, Renderer& renderer) override;
};

class RenderPipeline {
    // Also owns render targets
    std::vector<std::unique_ptr<RenderPass>> _passes;

public:
    void AddPass(std::unique_ptr<RenderPass> pass);
    void Render(GraphicsServer* ctx, Renderer& renderer);
};

class Renderer {
public:
    enum class RenderPath { Forward, Deferred };

    struct RenderTargetProps {
        int width = INIT_FRAMEBUFFER_WIDTH;
        int height = INIT_FRAMEBUFFER_HEIGHT;
        int numSamples = MSAA_NUM_SAMPLES;
    };

    Renderer() = default;
    ~Renderer() = default;

    void Init(int width, int height);
    void Cleanup();
    void Resize(int width, int height);

    // void ApplyMaterial(const Material& material);

    // void ApplyPipeline(const Pipeline& pipeline);

    // void ApplyCamera(const Camera& camera);

    // void ApplyTransform(const glm::mat4 transform);

    // void Draw(const std::shared_ptr<Mesh> mesh);

    void CheckFramebufferStatus(const std::string& prefix);
    void CheckErrors(const std::string& prefix);

    void PushDebugLine(DebugVertex from, DebugVertex to);

    void EnableWireframe(bool enable = true) {
        wireframeEnabled = enable;
    }

    void EnablePostProcess(bool enable = true) {
        postProcessEnabled = enable;
    }

    void SetCapability(const GLenum& cap, bool enable = true) {
        if (enable) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

    void SubmitOpaque(const RenderCommand& cmd);
    void SubmitVoxel(const RenderCommand& cmd);
    void SubmitTransparent(const RenderCommand& cmd);
    void SubmitGizmo(const RenderCommand& cmd);
    void SubmitHud(const RenderCommand& cmd);

    void RenderFrame(GraphicsServer* ctx, float dt);

    auto& GetOpaqueQueue() {
        return _opaqueQueue;
    }
    auto& GetTransparentQueue() {
        return _transparentQueue;
    }

    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    bool postProcessEnabled = false;
    bool wireframeEnabled = false;

    std::array<GLuint, MAX_UNI_LIGHTS> uniShadowMaps;
    std::array<GLuint, MAX_OMNI_LIGHTS> omniShadowMaps;
    GLuint envMap, irradianceMap;
    GLuint sceneColorTexture, sceneDepthTexture, sceneStencilTexture;
    GLuint msaaResolveTexture;

    struct GBuffer {
        GLuint id;
        GLuint positionRT;
        GLuint normalRT;
        GLuint albedoRT;
        GLuint materialRT;
        GLuint depthRT;
    } gBuffer;

    GLuint shadowFBO, sceneFBO, msaaResolveFBO;
    GLuint finalFBO = 0;

    GLuint debugVAO, debugVBO;
    GLuint canvasVAO, canvasVBO;
    GLuint screenVAO, screenVBO;

private:
    std::vector<RenderCommand> _opaqueQueue;
    std::vector<RenderCommand> _voxelQueue;
    std::vector<RenderCommand> _transparentQueue;
    std::vector<RenderCommand> _gizmoQueue;
    std::vector<RenderCommand> _hudQueue;

    std::unique_ptr<RenderPipeline> _pipeline;// TODO: support forward and deferred
    RenderPath _currRenderPath = RenderPath::Forward;

    void CreateFBOs();
    void DestroyFBOs();
    void CreateRTs(const RenderTargetProps& props);
    void DestroyRTs();

    void CreateCanvasVAO();
    void CreateScreenVAO();
    void CreateDebugVAO();
};