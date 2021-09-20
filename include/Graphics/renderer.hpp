#pragma once
#include "Globals.hpp"
#include "Framework.hpp"

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

class Renderer : Server
{
public:
    Renderer();

    ~Renderer();

    void Init(MessageBus* mb, Application* app);

    void Process(float dt) override;

    void OnMessage(Message msg) override;

    void CreateTexture(const std::string&);
    
    void BindSceneVAO();
    
    void BindScreenVAO();
    
    void BeginShadowPass();
    
    void EndShadowPass();
    
    void BeginColorPass();
    
    void EndColorPass();
    
    void BeginScreenColorPass();
    
    void EndScreenColorPass();

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
    std::vector<GLuint> textures;
    GLuint sceneVAO, screenVAO;
    GLuint screenVBO;

    void ResetFramebuffers();
    
    void ResetVertexArrays();
};