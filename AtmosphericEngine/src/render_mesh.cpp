#include "render_mesh.hpp"
#include "vertex.hpp"
#include "graphics_server.hpp"

RenderMesh::RenderMesh() {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);
}

RenderMesh::~RenderMesh() {
    Cleanup();
}

RenderMesh::RenderMesh(RenderMesh&& other) noexcept
    : _vao(other._vao)
    , _vbo(other._vbo)
    , _ebo(other._ebo)
    , _format(other._format)
    , _usage(other._usage)
    , _vertexCount(other._vertexCount)
    , _indexCount(other._indexCount)
    , _initialized(other._initialized)
    , _hasIndices(other._hasIndices)
{
    other._vao = 0;
    other._vbo = 0;
    other._ebo = 0;
    other._initialized = false;
}

RenderMesh& RenderMesh::operator=(RenderMesh&& other) noexcept {
    if (this != &other) {
        Cleanup();

        _vao = other._vao;
        _vbo = other._vbo;
        _ebo = other._ebo;
        _format = other._format;
        _usage = other._usage;
        _vertexCount = other._vertexCount;
        _indexCount = other._indexCount;
        _initialized = other._initialized;
        _hasIndices = other._hasIndices;

        other._vao = 0;
        other._vbo = 0;
        other._ebo = 0;
        other._initialized = false;
    }
    return *this;
}

void RenderMesh::Cleanup() {
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    if (_ebo != 0) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}

void RenderMesh::Initialize(VertexFormat format, BufferUsage usage) {
    _format = format;
    _usage = usage;

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    SetupVertexAttributes();

    glBindVertexArray(0);
    _initialized = true;
}

void RenderMesh::SetupVertexAttributes() {
    switch (_format) {
        case VertexFormat::Standard:
            // Vertex: position (vec3) + uv (vec2) + normal (vec3) + tangent (vec3) + bitangent (vec3)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(8 * sizeof(float)));
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(11 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            break;

        case VertexFormat::Debug:
            // DebugVertex: position (vec3) + color (vec3)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            break;

        case VertexFormat::Canvas:
            // CanvasVertex: position (vec2) + texCoord (vec2) + color (vec4) + texIndex (int) + layer (int)
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)(2 * sizeof(float)));
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CanvasVertex), (void*)(4 * sizeof(float)));
            glVertexAttribIPointer(3, 1, GL_INT, sizeof(CanvasVertex), (void*)(8 * sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            break;

        case VertexFormat::Voxel:
            // VoxelVertex: position (3x uint8) + voxel_id (uint8) + face_id (uint8)
            // All packed as integers, unpacked in vertex shader
            glVertexAttribIPointer(0, 3, GL_UNSIGNED_BYTE, sizeof(VoxelVertex), (void*)0);
            glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(VoxelVertex), (void*)(3 * sizeof(uint8_t)));
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(VoxelVertex), (void*)(4 * sizeof(uint8_t)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            break;
    }
}

GLenum RenderMesh::GetGLUsage() const {
    switch (_usage) {
        case BufferUsage::Static:  return GL_STATIC_DRAW;
        case BufferUsage::Dynamic: return GL_DYNAMIC_DRAW;
        case BufferUsage::Stream:  return GL_STREAM_DRAW;
        default: return GL_STATIC_DRAW;
    }
}

void RenderMesh::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize) {
    if (!_initialized) {
        Initialize(_format, _usage);
    }

    _vertexCount = vertexCount;
    _hasIndices = false;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertexData, GetGLUsage());
}

void RenderMesh::Upload(const void* vertexData, size_t vertexCount, size_t vertexSize,
                        const uint16_t* indexData, size_t indexCount) {
    if (!_initialized) {
        Initialize(_format, _usage);
    }

    _vertexCount = vertexCount;
    _indexCount = indexCount;
    _hasIndices = true;

    glBindVertexArray(_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertexData, GetGLUsage());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(uint16_t), indexData, GetGLUsage());

    glBindVertexArray(0);
}

void RenderMesh::Draw(GLenum primitiveType) const {
    glBindVertexArray(_vao);

    if (_hasIndices) {
        glDrawElements(primitiveType, static_cast<GLsizei>(_indexCount), GL_UNSIGNED_SHORT, nullptr);
    } else {
        glDrawArrays(primitiveType, 0, static_cast<GLsizei>(_vertexCount));
    }

    glBindVertexArray(0);
}
