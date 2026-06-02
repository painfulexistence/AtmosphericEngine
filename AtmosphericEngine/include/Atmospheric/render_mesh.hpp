#pragma once
#include "globals.hpp"
#include "gpu_buffer.hpp"

// Forward declarations for vertex types
struct Vertex;
struct DebugVertex;
struct CanvasVertex;

// OpenGL implementation of IGPUBuffer.
// Manages a VAO + VBO + optional EBO backed by the GL driver.
class RenderMesh : public IGPUBuffer {
public:
    RenderMesh();
    ~RenderMesh() override;

    RenderMesh(const RenderMesh&) = delete;
    RenderMesh& operator=(const RenderMesh&) = delete;
    RenderMesh(RenderMesh&& other) noexcept;
    RenderMesh& operator=(RenderMesh&& other) noexcept;

    // IGPUBuffer interface
    void Initialize(VertexFormat format, BufferUsage usage = BufferUsage::Static) override;
    void Upload(const void* vertexData, size_t vertexCount, size_t vertexSize) override;
    void Upload(
        const void* vertexData, size_t vertexCount, size_t vertexSize,
        const uint16_t* indexData, size_t indexCount) override;
    // ctx is unused for GL — pass nullptr.
    void Draw(
        IGPUCommandContext* ctx = nullptr,
        PrimitiveTopology topology = PrimitiveTopology::Triangles) const override;

    bool IsInitialized() const override { return _initialized; }
    size_t GetVertexCount() const override { return _vertexCount; }
    size_t GetIndexCount() const override { return _indexCount; }
    VertexFormat GetFormat() const override { return _format; }

    // OpenGL-specific convenience: draw with a raw GL primitive type.
    void Draw(GLenum primitiveType) const;

    GLuint GetVAO() const { return _vao; }

private:
    void SetupVertexAttributes();
    GLenum GetGLUsage() const;
    void Cleanup();

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    VertexFormat _format = VertexFormat::Standard;
    BufferUsage _usage = BufferUsage::Static;

    size_t _vertexCount = 0;
    size_t _indexCount = 0;
    bool _initialized = false;
    bool _hasIndices = false;
};
