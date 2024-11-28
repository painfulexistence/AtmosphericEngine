#pragma once
#include "globals.hpp"
#include "config.hpp"
#include "server.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "renderable.hpp"
#include "mesh.hpp"
#include "light.hpp"
#include "camera.hpp"

struct CanvasVertex {
    glm::vec2 position;
    glm::vec2 texCoord;
    glm::vec4 color;
    float texId;
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

struct RenderTargetProps
{
    RenderTargetProps(int width = INIT_FRAMEBUFFER_WIDTH, int height = INIT_FRAMEBUFFER_HEIGHT, int numSamples = MSAA_NUM_SAMPLES)
    {
        this->width = width;
        this->height = height;
        this->numSamples = numSamples;
    };
    int width;
    int height;
    int numSamples;
};

// class Pipeline;

// class ShaderProgram;

// class Texture;

// class Material;

class GraphicsServer : public Server
{
private:
    static GraphicsServer* _instance;

public:
    static GraphicsServer* Get()
    {
        return _instance;
    }
    std::vector<Mesh*> meshes;
    std::vector<GLuint> textures;
    std::vector<Material*> materials;
    std::vector<Renderable*> renderables;
    std::vector<Light*> lights;
    std::vector<Camera*> cameras;

    GraphicsServer();

    ~GraphicsServer();

    void Init(Application* app) override;

    void Process(float dt) override;

    void Render(float dt);

    void RenderUI(float dt);

    Camera* GetMainCamera() const {
        if (cameras.size() > 0) {
            return cameras[0];
        } else {
            return defaultCamera;
        }
    };

    Light* GetMainLight() const {
        if (lights.size() > 0) {
            return lights[0];
        } else {
            return defaultLight;
        }
    };

    Mesh* GetMesh(const std::string name) const {
        if (_namedMeshes.count(name) == 0)
            throw std::runtime_error("Could not find the specified mesh!");

        return _namedMeshes.find(name)->second;
    };

    void LoadTextures(const std::vector<std::string>& paths);

    void LoadDepthShader(const ShaderProgram& program);

    void LoadDepthCubemapShader(const ShaderProgram& program);

    void LoadColorShader(const ShaderProgram& program);

    void LoadDebugShader(const ShaderProgram& program);

    void LoadTerrainShader(const ShaderProgram& program);

    void LoadPostProcessShader(const ShaderProgram& program);

    void PushDebugLine(DebugVertex from, DebugVertex to);

    void ReloadShaders();

    void CheckErrors();

    void EnableWireframe(bool enable = true)
    {
        wireframeEnabled = enable;
    }

    void EnablePostProcess(bool enable = true)
    {
        postProcessEnabled = enable;
    }

    GLuint GetShadowMap(int lightType, int lightIdx = 0)
    {
        if (lightType == DIR_LIGHT)
        {
            return uniShadowMaps[lightIdx];
        }
        return omniShadowMaps[lightIdx];
    }

    void SetCapability(const GLenum& cap, bool enable = true)
    {
        if (enable) {
            glEnable(GL_CULL_FACE);
        } else {
            glDisable(GL_CULL_FACE);
        }
    }

    Material* CreateMaterial(Material* material = nullptr);

    Material* CreateMaterial(const std::string& name, Material* material = nullptr);

    Mesh* CreateMesh(Mesh* mesh = nullptr);

    Mesh* CreateMesh(const std::string& name, Mesh* mesh = nullptr);

    Mesh* CreateCubeMesh(const std::string& name, float size = 1.0f);

    Mesh* CreateSphereMesh(const std::string& name, float radius = 0.5f, int division = 18);

    Mesh* CreateCapsuleMesh(const std::string& name, float radius = 0.5f, float height = 3.0f);

    Mesh* CreateTerrainMesh(const std::string& name, float worldSize = 1024.f, int resolution = 10);

    Renderable* CreateRenderable(GameObject* gameObject, Mesh* mesh);

    Camera* CreateCamera(GameObject* gameObject, const CameraProps& props);

    Light* CreateLight(GameObject* gameObject, const LightProps& props);

    // void ApplyMaterial(const Material& material);

    // void ApplyPipeline(const Pipeline& pipeline);

    // void ApplyCamera(const Camera& camera);

    // void ApplyTransform(const glm::mat4 transform);

    // void Draw(const std::shared_ptr<Mesh> mesh);

private:
    GLuint shadowFBO, hdrFBO, msaaFBO;

    std::array<GLuint, MAX_UNI_LIGHTS> uniShadowMaps;
    std::array<GLuint, MAX_OMNI_LIGHTS> omniShadowMaps;
    GLuint hdrColorTexture, hdrDepthTexture, hdrStencilTexture;
    GLuint screenTexture;

    ShaderProgram colorShader;
    ShaderProgram depthShader;
    ShaderProgram depthCubemapShader;
    ShaderProgram terrainShader;
    ShaderProgram postProcessShader;
    ShaderProgram debugShader;

    GLuint canvasVAO, canvasVBO;
    GLuint screenVAO, screenVBO;
    GLuint debugVAO, debugVBO;

    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    std::map<std::string, Mesh*> _namedMeshes;
    std::map<std::string, Material*> _namedMaterials;
    std::map<Mesh*, std::vector<InstanceData>> _meshInstanceMap;
    // std::unordered_map<std::pair<Mesh*, Material*>, std::vector<InstanceData>> groupedObjects;

    std::vector<DebugVertex> debugLines;

    const int mainLightCount = 1;
    int auxLightCount = 0;
    bool postProcessEnabled = true;
    bool wireframeEnabled = false;
    Camera* defaultCamera = nullptr;
    Light* defaultLight = nullptr;

    void CreateRenderTargets(const RenderTargetProps& props);

    void UpdateRenderTargets(const RenderTargetProps& props);

    void CreateCanvasVAO();

    void CreateScreenVAO();

    void CreateDebugVAO();

    void ShadowPass(float dt);

    void ColorPass(float dt);

    void DebugPass(float dt);

    void MSAAPass(float dt);

    void PostProcessPass(float dt);
};