#pragma once
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>

namespace Rml = Rml;

class RmlUiRenderer : public Rml::RenderInterface {
public:
    RmlUiRenderer();
    ~RmlUiRenderer() override;

    // RenderInterface implementation
    void RenderGeometry(
        Rml::Vertex* vertices,
        int num_vertices,
        int* indices,
        int num_indices,
        Rml::TextureHandle texture,
        const Rml::Vector2f& translation
    ) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(int x, int y, int width, int height) override;

    bool LoadTexture(
        Rml::TextureHandle& texture_handle,
        Rml::Vector2i& texture_dimensions,
        const Rml::String& source
    ) override;

    bool GenerateTexture(
        Rml::TextureHandle& texture_handle,
        const Rml::byte* source,
        const Rml::Vector2i& source_dimensions
    ) override;

    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

    // Setup and rendering
    void Initialize();
    void Shutdown();
    void BeginFrame(int width, int height);
    void EndFrame();

private:
    struct TextureData {
        GLuint id;
        int width;
        int height;
    };

    struct Scissor {
        bool enabled = false;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };

    // OpenGL resources
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;
    GLuint m_shader_program = 0;

    // Texture management
    std::unordered_map<Rml::TextureHandle, TextureData> m_textures;
    Rml::TextureHandle m_next_texture_handle = 1;

    // State
    Scissor m_scissor;
    glm::mat4 m_transform = glm::mat4(1.0f);
    glm::mat4 m_projection = glm::mat4(1.0f);
    int m_viewport_width = 0;
    int m_viewport_height = 0;

    // Helper methods
    void CreateShaders();
    void DeleteShaders();
    GLuint CompileShader(GLenum type, const char* source);
    GLuint LinkProgram(GLuint vertex_shader, GLuint fragment_shader);
};
