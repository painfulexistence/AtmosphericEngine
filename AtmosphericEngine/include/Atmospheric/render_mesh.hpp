#pragma once
#include "globals.hpp"
#include <cstdint>
#include <vector>

// Forward declarations for vertex types
struct Vertex;
struct DebugVertex;
struct CanvasVertex;

// Voxel vertex - compact format for voxel chunk meshes
struct VoxelVertex {
    uint8_t x, y, z;     // Local position within chunk (0-255)
    uint8_t voxel_id;    // Voxel type
    uint8_t face_id;     // Face direction (0-5: +Y, -Y, +X, -X, +Z, -Z)
};

// Face direction enum for voxel meshes
enum class FaceDir : uint8_t {
    TOP = 0,     // +Y
    BOTTOM = 1,  // -Y
    RIGHT = 2,   // +X
    LEFT = 3,    // -X
    FRONT = 4,   // +Z
    BACK = 5     // -Z
};

// Vertex format descriptor
enum class VertexFormat {
    Standard,    // Vertex: pos, uv, normal, tangent, bitangent
    Debug,       // DebugVertex: pos, color
    Canvas,      // CanvasVertex: pos2D, texCoord, color, texIndex, layer
    Voxel        // VoxelVertex: pos (3x uint8), voxel_id, face_id
};

// Update frequency hint for buffer allocation
enum class BufferUsage {
    Static,      // GL_STATIC_DRAW - rarely updated
    Dynamic,     // GL_DYNAMIC_DRAW - updated occasionally
    Stream       // GL_STREAM_DRAW - updated every frame
};

// Handle type for RenderMesh references
struct RenderMeshHandle {
    static constexpr uint32_t INVALID = UINT32_MAX;
    uint32_t id = INVALID;

    bool IsValid() const { return id != INVALID; }
    bool operator==(const RenderMeshHandle& other) const { return id == other.id; }
    bool operator!=(const RenderMeshHandle& other) const { return id != other.id; }
};

// GPU resource wrapper - manages VAO/VBO/EBO
class RenderMesh {
public:
    RenderMesh();
    ~RenderMesh();

    // Non-copyable, movable
    RenderMesh(const RenderMesh&) = delete;
    RenderMesh& operator=(const RenderMesh&) = delete;
    RenderMesh(RenderMesh&& other) noexcept;
    RenderMesh& operator=(RenderMesh&& other) noexcept;

    // Initialize with vertex format (call once before Upload)
    void Initialize(VertexFormat format, BufferUsage usage = BufferUsage::Static);

    // Upload vertex data to GPU
    void Upload(const void* vertexData, size_t vertexCount, size_t vertexSize);

    // Upload with index buffer
    void Upload(const void* vertexData, size_t vertexCount, size_t vertexSize,
                const uint16_t* indexData, size_t indexCount);

    // Draw the mesh
    void Draw(GLenum primitiveType = GL_TRIANGLES) const;

    // Getters
    bool IsInitialized() const { return _initialized; }
    size_t GetVertexCount() const { return _vertexCount; }
    size_t GetIndexCount() const { return _indexCount; }
    VertexFormat GetFormat() const { return _format; }
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
