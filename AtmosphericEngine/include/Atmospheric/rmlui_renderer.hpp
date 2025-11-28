#pragma once
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

class RmlUiRenderer : public Rml::RenderInterface {
public:
    RmlUiRenderer();
    ~RmlUiRenderer() override;

    Rml::CompiledGeometryHandle
      CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
      override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

    void SetTransform(const Rml::Matrix4f* transform) override;

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

    struct CompiledGeometry {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
        int num_indices;
    };

    struct Scissor {
        bool enabled = false;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
    };

    // OpenGL resources
    GLuint m_shader_program = 0;

    // Texture management
    std::unordered_map<Rml::TextureHandle, TextureData> m_textures;
    Rml::TextureHandle m_next_texture_handle = 1;

    // Geometry management
    std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> m_geometry;
    Rml::CompiledGeometryHandle m_next_geometry_handle = 1;

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
