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
private:
    GLuint vbo, ebo;
    std::array<glm::vec3, 8> bounds;

public:
    static std::map<std::string, Mesh*> MeshList;

    GLuint vao;
    GLuint ibo;
    bool initialized = false;
    size_t vertCount;
    size_t triCount;

    MeshType type;
    Material* material;
    btCollisionShape* collisionShape;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum polygonMode = GL_FILL;

    Mesh();

    ~Mesh();

    void Initialize(MeshType type, const std::vector<Vertex>& verts);

    void Initialize(MeshType type, const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris);

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    static Mesh* CreateCube(const float& size = 1.0f);

    static Mesh* CreateSphere(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrain(const float& size = 1024.f, const int& resolution = 10);

    // static Mesh* CreateTerrain(const std::vector<float>& heightmap, const float& size = 1024.f, const int& resolution = 10);
};