#pragma once
#include "Globals.hpp"
#include "Shader.hpp"
#include "material.hpp"

class Mesh
{
private:
    GLuint vao, vbo, ebo, ibo;
    std::vector<GLfloat> verts;
    std::vector<GLushort> tris;
    std::array<glm::vec3, 8> bounds;
    std::list<std::uint64_t> _instances;
    bool _initialized = false;

    void BufferData();
    
public:
    Material material;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum drawMode = GL_FILL;

    Mesh();

    ~Mesh();

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    std::list<std::uint64_t> GetInstances() const { return _instances; }

    void AddInstance(const std::uint64_t& id) { _instances.push_back(id); }

    void RemoveInstance(const std::uint64_t& id) { _instances.remove(id); }

    void AsCube(const GLfloat& size = 1.0f);

    void AsSphere(const GLfloat& radius = 0.5f, const GLint& division = 18);

    void AsTerrain(const GLfloat& size, const GLint& vnum, const std::vector<GLfloat>& heightmap);

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;
};