#pragma once
#include "globals.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "bullet_collision.hpp"
#include "vertex.hpp"
#include <cstdint>
#include <vector>

enum class MeshType
{
    PRIM = 0,
    TERRAIN = 1,
    SKY = 2
};

class Mesh
{
public:
    MeshType type;
    size_t vertCount;
    size_t triCount;
    bool initialized = false;
    GLuint vao;
    GLuint ibo;

    Mesh(MeshType type = MeshType::PRIM);

    ~Mesh();

    void Initialize(const std::vector<Vertex>& verts);

    void Initialize(const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris);

    std::array<glm::vec3, 8> GetBoundingBox() const { return _bounds; }

    void SetBoundingBox(const std::array<glm::vec3, 8> bounds) { _bounds = bounds; }

    Material* GetMaterial() const { return _material; }

    void SetMaterial(Material* material) { _material = material; };

    btCollisionShape* GetShape() { return _shape; }

    void SetShape(btCollisionShape* shape) { _shape = shape; }

    void SetShapeLocalScaling(glm::vec3 localScaling);

    void AddCapsuleShape(float radius, float height);

private:
    GLuint vbo, ebo;
    std::array<glm::vec3, 8> _bounds;

    Material* _material;
    btCollisionShape* _shape;
};

class MeshBuilder {
public:
    static Mesh* CreateCube(const float& size = 1.0f);

    static Mesh* CreateSphere(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrain(const float& size = 1024.f, const int& resolution = 10);

    // static Mesh* CreateTerrain(const std::vector<float>& heightmap, const float& size = 1024.f, const int& resolution = 10);

    static Mesh* CreateCubeWithPhysics(const float& size = 1.0f);

    static Mesh* CreateSphereWithPhysics(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrainWithPhysics(const float& size = 1024.f, const int& resolution = 10, const std::string& heightmap = "assets/textures/heightmap_debug.jpg");

    void PushQuad();

    void PushCube();

    std::shared_ptr<Mesh> Build();

private:
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
};