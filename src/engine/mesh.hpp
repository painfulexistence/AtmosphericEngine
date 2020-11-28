#pragma once
#include "../common.hpp"
#include "material.hpp"
#include "program.hpp"

class Mesh
{
private:
    GLuint vao, vbo, ebo, ibo;
    std::vector<GLfloat> verts;
    std::vector<GLushort> tris;
    Material _mat;
    std::list<std::uint64_t> _instances;
    GLenum _primitiveType = GL_TRIANGLES;
    bool _initialized = false;

    void BufferData();
    
public:
    bool cullFaceEnabled = true;
    GLenum drawMode = GL_FILL;

    Mesh();

    ~Mesh();

    Material GetMaterial() const { return _mat; }

    void SetMaterial(const Material& mat) { _mat = mat; }

    std::list<std::uint64_t> GetInstances() const { return _instances; }

    void AddInstance(const std::uint64_t& id) { _instances.push_back(id); }

    void RemoveInstance(const std::uint64_t& id) { _instances.remove(id); }

    void AsCube(const GLfloat& size = 1.0f);

    void AsSphere(const GLfloat& radius = 0.5f, const GLint& division = 18);

    void AsTerrain(const GLfloat& size, const GLint& vnum, const std::vector<GLfloat>& heightmap);

    void Render(Program& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(Program& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;
};