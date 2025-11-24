#pragma once
#include "bullet_collision.hpp"
#include "globals.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "vertex.hpp"
#include <cstdint>
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

    // Dynamic update methods for per-frame geometry
    template<typename VertexType>
    void UpdateDynamic(const std::vector<VertexType>& verts, GLenum primType = GL_TRIANGLES);

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

    std::shared_ptr<Mesh> Build();

private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};