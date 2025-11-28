#include "rmlui_renderer.hpp"
#include <RmlUi/Core.h>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    // Setup vertex attributes
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Position (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, position));

    // Color (vec4 as bytes)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, colour));

    // TexCoord (vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, tex_coord));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

    glBindVertexArray(0);

    CreateShaders();

    spdlog::info("RmlUi renderer initialized");
}

void RmlUiRenderer::Shutdown() {
    // Delete textures
    for (auto& pair : m_textures) {
        glDeleteTextures(1, &pair.second.id);
    }
    m_textures.clear();

    // Delete OpenGL resources
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    DeleteShaders();

    m_vao = 0;
    m_vbo = 0;
    m_ebo = 0;
}

void RmlUiRenderer::BeginFrame(int width, int height) {
    m_viewport_width = width;
    m_viewport_height = height;

    // Create orthographic projection matrix for 2D rendering
    m_projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}

void RmlUiRenderer::EndFrame() {
    // Nothing to do here for now
}

void RmlUiRenderer::RenderGeometry(
    Rml::Vertex* vertices,
    int num_vertices,
    int* indices,
    int num_indices,
    Rml::TextureHandle texture,
    const Rml::Vector2f& translation
) {
    if (num_vertices == 0 || num_indices == 0) return;

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
        auto it = m_textures.find(texture);
        if (it != m_textures.end()) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, it->second.id);
            GLint texture_loc = glGetUniformLocation(m_shader_program, "uTexture");
            glUniform1i(texture_loc, 0);
        }
    }

    // Setup scissor test
    if (m_scissor.enabled) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_scissor.x, m_viewport_height - (m_scissor.y + m_scissor.height),
                  m_scissor.width, m_scissor.height);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }

    // Upload vertex data
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(Rml::Vertex), vertices, GL_STREAM_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(int), indices, GL_STREAM_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_SCISSOR_TEST);
}

void RmlUiRenderer::EnableScissorRegion(bool enable) {
    m_scissor.enabled = enable;
}

void RmlUiRenderer::SetScissorRegion(int x, int y, int width, int height) {
    m_scissor.x = x;
    m_scissor.y = y;
    m_scissor.width = width;
    m_scissor.height = height;
}

bool RmlUiRenderer::LoadTexture(
    Rml::TextureHandle& texture_handle,
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source
) {
    // For now, we'll just log that a texture was requested
    // In a full implementation, you'd load the image file here
    spdlog::warn("RmlUi texture loading not fully implemented: {}", source);
    texture_handle = 0;
    return false;
}

bool RmlUiRenderer::GenerateTexture(
    Rml::TextureHandle& texture_handle,
    const Rml::byte* source,
    const Rml::Vector2i& source_dimensions
) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, source);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture_handle = m_next_texture_handle++;
    m_textures[texture_handle] = { texture_id, source_dimensions.x, source_dimensions.y };

    return true;
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
        m_transform = glm::make_mat4(&(*transform)(0, 0));
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
