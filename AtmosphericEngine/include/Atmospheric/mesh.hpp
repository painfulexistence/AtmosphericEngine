#pragma once
#include "bullet_collision.hpp"
#include "csg.hpp"
#include "globals.hpp"
#include "material.hpp"
#include "render_mesh.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include <cstdint>
#include <glm/gtc/quaternion.hpp>
#include <vector>

enum class MeshType {
    PRIM = 0,
    TERRAIN = 1,
    SKY = 2,
    DEBUG = 3,// Debug lines, wireframes
    CANVAS = 4// UI/Canvas elements
};

enum class UpdateFrequency {
    Static,// One-time upload, never changes (normal 3D models)
    Dynamic,// May change per frame (debug lines, canvas, particles)
    Stream// Always changes per frame
};

class Mesh {
public:
    MeshType type;
    UpdateFrequency updateFreq = UpdateFrequency::Static;
    size_t vertCount;
    size_t triCount;
    bool initialized = false;
    GLuint vao;
    GLuint ibo;

    Mesh(MeshType type = MeshType::PRIM);

    ~Mesh();

    void Initialize(const std::vector<Vertex>& verts);

    void Initialize(const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris);

    // Dynamic update methods for per-frame geometry (legacy)
    template<typename VertexType>
    void UpdateDynamic(const std::vector<VertexType>& verts, GLenum primType = GL_TRIANGLES);

    // Update with voxel vertex data (uses internal RenderMesh)
    void Update(const std::vector<VoxelVertex>& vertices);

    // Check if this mesh uses the new RenderMesh system
    bool UsesRenderMesh() const { return _renderMeshHandle.IsValid(); }

    // Get the RenderMesh handle (for rendering)
    RenderMeshHandle GetRenderMeshHandle() const { return _renderMeshHandle; }

    GLenum GetPrimitiveType() const {
        return _primitiveType;
    }

    std::array<glm::vec3, 8> GetBoundingBox() const {
        return _bounds;
    }

    void SetBoundingBox(const std::array<glm::vec3, 8> bounds) {
        _bounds = bounds;
    }

    Material* GetMaterial() const {
        return _material;
    }

    void SetMaterial(Material* material) {
        _material = material;
    };

    btCollisionShape* GetShape() {
        return _shape;
    }

    void SetShape(btCollisionShape* shape) {
        _shape = shape;
    }

    void SetShapeLocalScaling(glm::vec3 localScaling);

    void AddCapsuleShape(float radius, float height);

private:
    GLuint vbo, ebo;
    GLenum _primitiveType = GL_TRIANGLES;
    std::array<glm::vec3, 8> _bounds;

    Material* _material;
    btCollisionShape* _shape;

    // New RenderMesh-based storage (used by Update methods)
    RenderMeshHandle _renderMeshHandle;

    template<typename VertexType> void InitializeDynamic(GLenum primType);
};

class MeshBuilder {
public:
    static Mesh* CreateCube(const float& size = 1.0f);

    static Mesh* CreateSphere(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrain(const float& size = 1024.f, const int& resolution = 10);

    // static Mesh* CreateTerrain(const std::vector<float>& heightmap, const float& size = 1024.f, const int& resolution
    // = 10);

    static Mesh* CreateCubeWithPhysics(const float& size = 1.0f);

    static Mesh* CreateSphereWithPhysics(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrainWithPhysics(
      const float& size = 1024.f,
      const int& resolution = 10,
      const std::string& heightmap = "assets/textures/heightmap_debug.jpg"
    );

    void PushQuad(
      glm::vec3 position,
      glm::vec2 size,
      glm::vec3 normal,
      glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
      glm::vec2 uvMin = glm::vec2(0.0f),
      glm::vec2 uvMax = glm::vec2(1.0f)
    );

    void PushCube(glm::vec3 position, glm::vec3 size, glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f));

    // Push CSG compiled boxes
    void PushCSG(const std::vector<CSG::AABB>& boxes);

    std::shared_ptr<Mesh> Build();

    void Clear();

private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};

// Builder for voxel chunk meshes using compact VoxelVertex format
class VoxelMeshBuilder {
public:
    // Push a single voxel face
    // pos: local position within chunk (0-255 range)
    // dir: face direction
    // voxelId: voxel type identifier
    void PushFace(glm::ivec3 pos, FaceDir dir, uint8_t voxelId);

    // Push a full cube (all 6 faces) - use when all faces are visible
    void PushCube(glm::ivec3 pos, uint8_t voxelId);

    // Build and return vertex data
    const std::vector<VoxelVertex>& Build() { return _vertices; }

    // Get vertex count
    size_t GetVertexCount() const { return _vertices.size(); }

    // Clear accumulated data
    void Clear();

private:
    std::vector<VoxelVertex> _vertices;
};