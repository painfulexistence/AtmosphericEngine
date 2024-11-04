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

struct DebugVertex {
    glm::vec3 position;
    glm::vec3 color;
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

class GraphicsServer : Server
{
private:
    static GraphicsServer* _instance;

public:
    static GraphicsServer* Get()
    {
        return _instance;
    }

    std::vector<GLuint> textures;
    std::vector<Material*> materials;
    std::vector<Renderable*> renderables;
    std::vector<Light*> lights;
    std::vector<Camera*> cameras;

    GraphicsServer();

    ~GraphicsServer();

    void Init(Application* app);

    void Process(float dt) override;

    void Render(float dt);

    void RenderUI(float dt);

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

    GLuint screenVAO, screenVBO;
    GLuint debugVAO, debugVBO;

    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    std::map<Mesh*, std::vector<glm::mat4>> meshInstances;

    std::vector<DebugVertex> debugLines;

    const int mainLightCount = 1;
    int auxLightCount = 0;
    bool postProcessEnabled = true;
    bool wireframeEnabled = false;

    void CreateRenderTargets(const RenderTargetProps& props);

    void UpdateRenderTargets(const RenderTargetProps& props);

    void CreateDebugVAO();

    void CreateScreenVAO();

    void ShadowPass(float dt);

    void ColorPass(float dt);

    void DebugPass(float dt);

    void MSAAPass(float dt);

    void PostProcessPass(float dt);
};