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

enum class DrawMode { Static, Dynamic, Stream };

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

// class Pipeline;

// class ShaderProgram;

// class Texture;

// class Material;

class Renderer;

class MeshComponent;
class SpriteComponent;
class CameraComponent;
class LightComponent;

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

    std::vector<DebugVertex> debugLines;
    std::vector<CanvasVertex> canvasDrawList;
    int _debugLineCount = 0;
    int _canvasQuadCount = 0;

    Mesh* debugLineMesh = nullptr;
    Mesh* canvasMesh = nullptr;
    ShaderProgram* debugShader = nullptr;
    ShaderProgram* canvasShader = nullptr;

    GraphicsServer();
    ~GraphicsServer();

    void Init(Application* app) override;
    void Process(float dt) override;
    void DrawImGui(float dt) override;

    void Reset();
    void Render(CameraComponent* camera, float dt);

    void PushDebugLine(DebugVertex from, DebugVertex to) {
        debugLines.push_back(from);
        debugLines.push_back(to);
    }

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

    MeshComponent* RegisterMesh(MeshComponent* mesh);
    CameraComponent* RegisterCamera(CameraComponent* camera);
    LightComponent* RegisterLight(LightComponent* light);
    SpriteComponent* RegisterSprite(SpriteComponent* sprite);

    Renderer* renderer = nullptr;

private:
    CameraComponent* defaultCamera = nullptr;
    LightComponent* defaultLight = nullptr;

    static constexpr int MAX_CANVAS_TEXTURES = 32;

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
};