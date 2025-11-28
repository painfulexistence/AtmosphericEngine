#include "rmlui_renderer.hpp"
#include <RmlUi/Core.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

// Vertex shader for RmlUi rendering
static const char* VERTEX_SHADER_SOURCE = R"(
#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uTransform;

out vec4 vColor;
out vec2 vTexCoord;

void main() {
    vColor = aColor;
    vTexCoord = aTexCoord;
    gl_Position = uProjection * uTransform * vec4(aPosition, 0.0, 1.0);
}
)";

// Fragment shader for RmlUi rendering
static const char* FRAGMENT_SHADER_SOURCE = R"(
#version 330 core
in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform bool uHasTexture;

out vec4 FragColor;

void main() {
    if (uHasTexture) {
        FragColor = vColor * texture(uTexture, vTexCoord);
    } else {
        FragColor = vColor;
    }
}
)";

RmlUiRenderer::RmlUiRenderer() {
}

RmlUiRenderer::~RmlUiRenderer() {
    Shutdown();
}

void RmlUiRenderer::Initialize() {
    CreateShaders();
    spdlog::info("RmlUi renderer initialized");
}

void RmlUiRenderer::Shutdown() {
    for (auto& pair : m_textures) {
        glDeleteTextures(1, &pair.second.id);
    }
    m_textures.clear();

    DeleteShaders();
}

void RmlUiRenderer::BeginFrame(int width, int height) {
    m_viewport_width = width;
    m_viewport_height = height;
    m_projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void RmlUiRenderer::EndFrame() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

Rml::CompiledGeometryHandle
  RmlUiRenderer::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Rml::Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, position));

    // Color (vec4 as bytes)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, colour));

    // TexCoord (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, tex_coord));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    Rml::CompiledGeometryHandle handle = m_next_geometry_handle++;
    m_geometry[handle] = { vao, vbo, ebo, (int)indices.size() };

    return handle;
}

void RmlUiRenderer::RenderGeometry(
  Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture
) {
    auto it = m_geometry.find(geometry);
    if (it == m_geometry.end()) return;

    const CompiledGeometry& geom = it->second;

    // Enable blending for UI rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind shader
    glUseProgram(m_shader_program);

    // Set uniforms
    GLint proj_loc = glGetUniformLocation(m_shader_program, "uProjection");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(m_projection));

    // Apply translation to transform
    glm::mat4 transform = m_transform;
    transform = glm::translate(transform, glm::vec3(translation.x, translation.y, 0.0f));

    GLint transform_loc = glGetUniformLocation(m_shader_program, "uTransform");
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));

    // Bind texture if provided
    bool has_texture = (texture != 0);
    GLint has_texture_loc = glGetUniformLocation(m_shader_program, "uHasTexture");
    glUniform1i(has_texture_loc, has_texture);

    if (has_texture) {
        auto tex_it = m_textures.find(texture);
        if (tex_it != m_textures.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_it->second.id);
            GLint texture_loc = glGetUniformLocation(m_shader_program, "uTexture");
            glUniform1i(texture_loc, 0);
        }
    }

    // Setup scissor test
    if (m_scissor.enabled) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_scissor.x, m_viewport_height - (m_scissor.y + m_scissor.height), m_scissor.width, m_scissor.height);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }

    // Draw
    glBindVertexArray(geom.vao);
    glDrawElements(GL_TRIANGLES, geom.num_indices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glUseProgram(0);
    glDisable(GL_SCISSOR_TEST);
}

void RmlUiRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    auto it = m_geometry.find(geometry);
    if (it != m_geometry.end()) {
        glDeleteVertexArrays(1, &it->second.vao);
        glDeleteBuffers(1, &it->second.vbo);
        glDeleteBuffers(1, &it->second.ebo);
        m_geometry.erase(it);
    }
}

void RmlUiRenderer::EnableScissorRegion(bool enable) {
    m_scissor.enabled = enable;
}

void RmlUiRenderer::SetScissorRegion(Rml::Rectanglei region) {
    m_scissor.x = region.Left();
    m_scissor.y = region.Top();
    m_scissor.width = region.Width();
    m_scissor.height = region.Height();
}

Rml::TextureHandle RmlUiRenderer::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    // For now, we'll just log that a texture was requested
    // In a full implementation, you'd load the image file here
    spdlog::warn("RmlUi texture loading not fully implemented: {}", source);
    return 0;
}

Rml::TextureHandle RmlUiRenderer::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, source.data()
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    Rml::TextureHandle texture_handle = m_next_texture_handle++;
    m_textures[texture_handle] = { texture_id, source_dimensions.x, source_dimensions.y };

    return texture_handle;
}

void RmlUiRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
    auto it = m_textures.find(texture_handle);
    if (it != m_textures.end()) {
        glDeleteTextures(1, &it->second.id);
        m_textures.erase(it);
    }
}

void RmlUiRenderer::SetTransform(const Rml::Matrix4f* transform) {
    if (transform) {
        // Convert RmlUi matrix to glm matrix
        // RmlUi uses column-major ordering like OpenGL
        m_transform = glm::make_mat4(transform->data());
    } else {
        m_transform = glm::mat4(1.0f);
    }
}

void RmlUiRenderer::CreateShaders() {
    GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SOURCE);
    GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SOURCE);
    m_shader_program = LinkProgram(vertex_shader, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void RmlUiRenderer::DeleteShaders() {
    if (m_shader_program) {
        glDeleteProgram(m_shader_program);
        m_shader_program = 0;
    }
}

GLuint RmlUiRenderer::CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        spdlog::error("Shader compilation failed: {}", info_log);
    }

    return shader;
}

GLuint RmlUiRenderer::LinkProgram(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, nullptr, info_log);
        spdlog::error("Shader program linking failed: {}", info_log);
    }

    return program;
}
