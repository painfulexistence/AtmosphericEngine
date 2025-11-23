#pragma once
#include "camera_component.hpp"
#include "config.hpp"
#include "light_component.hpp"
#include "mesh.hpp"
#include "mesh_component.hpp"
#include "server.hpp"
#include "shader.hpp"
#include "sprite_component.hpp"
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <unordered_map>


enum class RenderPath {
    Forward,
    Deferred
};

struct CanvasVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
    glm::vec4 color;
    int texIndex;
};

struct ScreenVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
};

struct DebugVertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct CameraData {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

struct InstanceData {
    glm::mat4 modelMatrix;
};

struct RenderTargetProps {
    int width = INIT_FRAMEBUFFER_WIDTH;
    int height = INIT_FRAMEBUFFER_HEIGHT;
    int numSamples = MSAA_NUM_SAMPLES;
};

// class Pipeline;

// class ShaderProgram;

// class Texture;

// class Material;

class GraphicsServer : public Server {
private:
    static GraphicsServer* _instance;

public:
    static GraphicsServer* Get() {
        return _instance;
    }
    std::vector<GLuint> canvasTextures;// TODO: Replace with AssetManager
    std::vector<MeshComponent*> renderables;
    std::vector<SpriteComponent*> canvasDrawables;
    std::vector<LightComponent*> directionalLights;
    std::vector<LightComponent*> pointLights;
    std::vector<CameraComponent*> cameras;

    GraphicsServer();
    ~GraphicsServer();

    void Init(Application* app) override;
    void Process(float dt) override;
    void DrawImGui(float dt) override;

    void Reset();
    void Render(float dt);

    CameraComponent* GetMainCamera() const {
        if (cameras.size() > 0) {
            return cameras[0];
        } else {
            return defaultCamera;
        }
    };

    LightComponent* GetMainLight() const {
        if (directionalLights.size() > 0) {
            return directionalLights[0];
        } else {
            return defaultLight;
        }
    };

    ShaderProgram* GetShader(const std::string& name) const;
    ShaderProgram* GetShaderByID(uint32_t id) const;
    Mesh* GetMesh(const std::string& name) const;

    void CheckFramebufferStatus(const std::string& prefix);
    void CheckErrors(const std::string& prefix);

    void PushDebugLine(DebugVertex from, DebugVertex to);

    void PushCanvasQuad(
      float x,
      float y,
      float w,
      float h,
      float angle,
      float pivotX,
      float pivotY,
      const glm::vec4& color,
      int texIndex,
      const glm::vec2& uvMin = glm::vec2(0.0f),
      const glm::vec2& uvMax = glm::vec2(1.0f)
    );
    void PushCanvasQuadTiled(
      float x,
      float y,
      float w,
      float h,
      float angle,
      float pivotX,
      float pivotY,
      const glm::vec4& color,
      int texIndex,
      const glm::vec2& tilesetSize,
      const glm::vec2& tileIndex
    );

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


    MeshComponent* RegisterMesh(MeshComponent* mesh);
    CameraComponent* RegisterCamera(CameraComponent* camera);
    LightComponent* RegisterLight(LightComponent* light);
    SpriteComponent* RegisterSprite(SpriteComponent* sprite);

    // void ApplyMaterial(const Material& material);

    // void ApplyPipeline(const Pipeline& pipeline);

    // void ApplyCamera(const Camera& camera);

    // void ApplyTransform(const glm::mat4 transform);

    // void Draw(const std::shared_ptr<Mesh> mesh);

    void SetRenderPath(RenderPath path) {
        _currRenderPath = path;
    }
    RenderPath GetRenderPath() const {
        return _currRenderPath;
    }

private:
    RenderPath _currRenderPath = RenderPath::Forward;

    GLuint debugVAO, debugVBO;
    GLuint canvasVAO, canvasVBO;
    GLuint screenVAO, screenVBO;

    std::array<GLuint, MAX_UNI_LIGHTS> uniShadowMaps;
    std::array<GLuint, MAX_OMNI_LIGHTS> omniShadowMaps;
    GLuint sceneColorTexture, sceneDepthTexture, sceneStencilTexture;
    GLuint msaaResolveTexture;


    GLuint shadowFBO, sceneFBO, msaaResolveFBO;
    GLuint finalFBO = 0;
    struct GBuffer {
        GLuint id;
        GLuint positionRT;
        GLuint normalRT;
        GLuint albedoRT;
        GLuint materialRT;
        GLuint depthRT;
    } gBuffer;

    std::unordered_map<Mesh*, std::vector<InstanceData>> _meshInstanceMap;
    // std::unordered_map<std::pair<Mesh*, Material*>, std::vector<InstanceData>> groupedObjects;

    std::vector<DebugVertex> debugLines;
    std::vector<CanvasVertex> canvasDrawList;

    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    bool postProcessEnabled = false;
    bool wireframeEnabled = false;

    CameraComponent* defaultCamera = nullptr;
    LightComponent* defaultLight = nullptr;

    int auxLightCount = 0;
    int _canvasQuadCount = 0;
    int _debugLineCount = 0;

    void CreateFBOs();
    void DestroyFBOs();
    void CreateRTs(const RenderTargetProps& props);
    void DestroyRTs();

    void CreateCanvasVAO();
    void CreateScreenVAO();
    void CreateDebugVAO();

    void ShadowPass(float dt);
    void ForwardPass(float dt);
    void MSAAResolvePass(float dt);
    void CanvasPass(float dt);
    void PostProcessPass(float dt);

    void GeometryPass(float dt);
    void LightingPass(float dt);

    static constexpr int MAX_CANVAS_TEXTURES = 32;
};