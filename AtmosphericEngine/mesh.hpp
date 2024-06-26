#pragma once
#include "globals.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "bullet_collision.hpp"
#include "vertex.hpp"
#include <cstdint>

class Mesh
{
private:
    GLuint vao, vbo, ebo, ibo;
    size_t vertCount;
    size_t triCount;
    std::array<glm::vec3, 8> bounds;
    bool _initialized = false;

public:
    static std::map<std::string, Mesh*> MeshList;

    Material* material;
    btCollisionShape* collisionShape;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum drawMode = GL_FILL;

    Mesh();

    ~Mesh();

    void Initialize(const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris);

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;

    static Mesh* CreateCube(const float& size = 1.0f);

    static Mesh* CreateSphere(const float& radius = 0.5f, const int& division = 18);

    static Mesh* CreateTerrain(const float& size, const int& vnum, const std::vector<float>& heightmap);
};