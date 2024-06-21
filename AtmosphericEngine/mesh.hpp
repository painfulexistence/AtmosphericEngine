#pragma once
#include "globals.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "bullet_collision.hpp"

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

    void Initialize(const std::vector<GLfloat>& verts, const std::vector<GLushort>& tris);

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;

    static Mesh* CreateCube(const GLfloat& size = 1.0f);

    static Mesh* CreateSphere(const GLfloat& radius = 0.5f, const GLint& division = 18);

    static Mesh* CreateTerrain(const GLfloat& size, const GLint& vnum, const std::vector<GLfloat>& heightmap);
};