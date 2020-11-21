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

    bool _initialized = false;
    GLenum _primitiveType = GL_TRIANGLES;
    GLenum _drawMode = GL_FILL;
    GLenum _cullFaceMode = GL_BACK;

    void BufferData();
    
public:
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

    void SetDrawMode(GLenum mode) { _drawMode = mode; }

    void SetCullFaceMode(GLenum mode) { _cullFaceMode = mode; }

    void Render(Program& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(Program& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;
};