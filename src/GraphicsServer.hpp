#pragma once
#include "Globals.hpp"
#include "Server.hpp"
#include "Window.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "light.hpp"
#include "Camera.hpp"

struct FramebufferProps
{
    FramebufferProps(int width = INIT_FRAMEBUFFER_WIDTH, int height = INIT_FRAMEBUFFER_HEIGHT, int numSapmples = MSAA_NUM_SAMPLES)
    {
        this->width = width;
        this->height = height;
        this->numSapmples = numSapmples;
    };
    int width;
    int height;
    int numSapmples;
};

class GraphicsServer : Server
{
public:
    std::vector<GLuint> textures;
    std::vector<Material*> materials;
    std::vector<Mesh*> meshes;
    std::vector<Light*> lights;
    std::vector<Camera*> cameras;
    ShaderProgram colorProgram;
    ShaderProgram depthTextureProgram;
    ShaderProgram depthCubemapProgram;
    ShaderProgram hdrProgram;
    
    GraphicsServer();

    ~GraphicsServer();

    void Init(MessageBus* mb, Application* app);

    void Process(float dt) override;

    void Render(float dt);

    void OnMessage(Message msg) override;
    
    void LoadTexture(const std::string& path);

    void LoadTextures(const std::vector<std::string>& paths);

    void LoadShaders(const std::vector<std::string>& paths);

    void CheckErrors();

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
    FramebufferProps _fbProps;
    GLuint shadowFBO, hdrFBO, msaaFBO;
    std::vector<GLuint> uniShadowMaps;
    std::vector<GLuint> omniShadowMaps;
    GLuint hdrColorTexture, hdrDepthTexture;
    GLuint screenTexture;
    GLuint screenVAO;
    GLuint screenVBO;
    glm::vec4 clearColor = glm::vec4(0.15f, 0.183f, 0.2f, 1.0f);
    std::map<Model*, std::vector<glm::mat4>> modelInstancesMap;
    const int mainLightCount = 1;
    int auxLightCount = 0;

    void ResetFramebuffers();
    
    void ResetScreenVAO();

    void ShadowPass(float dt);

    void ColorPass(float dt);

    void PostProcessPass(float dt);
};