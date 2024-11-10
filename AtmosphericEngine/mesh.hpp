#pragma once
#include "globals.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "bullet_collision.hpp"
#include "vertex.hpp"
#include <cstdint>

enum MeshType
{
    MESH_PRIM = 0,
    MESH_TERRAIN = 1,
    MESH_SKY = 2
};

class Mesh
{
public:
    static std::map<std::string, Mesh*> MeshList;

    GLuint vao;
    GLuint ibo;
    bool initialized = false;
    size_t vertCount;
    size_t triCount;

    MeshType type;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum polygonMode = GL_FILL;

    Mesh();

    ~Mesh();

    void Initialize(MeshType type, const std::vector<Vertex>& verts);

    void Initialize(MeshType type, const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris);

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    Material* GetMaterial() { return _material; }

    void SetMaterial(Material* material);

    void AddCapsuleShape(float radius, float height);

    btCollisionShape* GetShape() { return _shape; }

    void SetShape(btCollisionShape* shape) { _shape = shape; }

    static Mesh* CreateCube(const float& size = 1.0f);

    static Mesh* CreateSphere(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrain(const float& size = 1024.f, const int& resolution = 10);

    // static Mesh* CreateTerrain(const std::vector<float>& heightmap, const float& size = 1024.f, const int& resolution = 10);

    static Mesh* CreateCubeWithPhysics(const float& size = 1.0f);

    static Mesh* CreateSphereWithPhysics(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrainWithPhysics(const float& size = 1024.f, const int& resolution = 10, const std::string& heightmap = "assets/textures/heightmap_debug.jpg");

private:
    GLuint vbo, ebo;
    std::array<glm::vec3, 8> bounds;

    Material* _material;
    btCollisionShape* _shape;
};