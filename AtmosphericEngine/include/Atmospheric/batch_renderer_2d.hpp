#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

// Forward declarations
class ShaderProgram;

struct BatchVertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
    float texIndex;
    float entityID;// For picking or other identification
};

struct BatchStats {
    uint32_t drawCalls = 0;
    uint32_t quadCount = 0;
};

class BatchRenderer2D {
public:
    BatchRenderer2D();
    ~BatchRenderer2D();

    void Init();
    void Shutdown();

    void BeginScene(const glm::mat4& viewProj);
    void EndScene();
    void Flush();

    // Primitives
    void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    void DrawQuad(
      const glm::vec2& position, const glm::vec2& size, uint32_t textureID, const glm::vec4& color = glm::vec4(1.0f)
    );
    void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
    void DrawQuad(
      const glm::vec3& position, const glm::vec2& size, uint32_t textureID, const glm::vec4& color = glm::vec4(1.0f)
    );

    // Rotated Primitives
    void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
    void DrawRotatedQuad(
      const glm::vec2& position,
      const glm::vec2& size,
      float rotation,
      uint32_t textureID,
      const glm::vec4& color = glm::vec4(1.0f)
    );
    void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
    void DrawRotatedQuad(
      const glm::vec3& position,
      const glm::vec2& size,
      float rotation,
      uint32_t textureID,
      const glm::vec4& color = glm::vec4(1.0f)
    );

    // Advanced
    void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
    void DrawQuad(
      const glm::mat4& transform, uint32_t textureID, const glm::vec4& color = glm::vec4(1.0f), int entityID = -1
    );

    // With UVs (for spritesheets)
    void DrawQuad(
      const glm::mat4& transform,
      uint32_t textureID,
      const glm::vec2* texCoords,
      const glm::vec4& color = glm::vec4(1.0f),
      int entityID = -1
    );


    BatchStats GetStats();
    void ResetStats();

private:
    void StartBatch();
    void NextBatch();

    struct Renderer2DData;
    std::unique_ptr<Renderer2DData> m_Data;
};
