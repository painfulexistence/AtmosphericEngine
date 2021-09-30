#pragma once
#include "Globals.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/material.hpp"
#include "Physics/BulletCollision.hpp"

class Model
{
private:
    GLuint vao, vbo, ebo, ibo;
    std::vector<GLfloat> verts;
    std::vector<GLushort> tris;
    std::array<glm::vec3, 8> bounds;
    bool _initialized = false;
    
public:
    static std::map<std::string, Model*> ModelList;

    Material* material;
    btCollisionShape* collisionShape;
    bool cullFaceEnabled = true;
    GLenum primitiveType = GL_TRIANGLES;
    GLenum drawMode = GL_FILL;

    Model();

    ~Model();

    void BufferData();

    std::array<glm::vec3, 8> GetBoundingBox() const { return bounds; }

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const;

    void Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const;

    static Model* CreateCube(const GLfloat& size = 1.0f);

    static Model* CreateSphere(const GLfloat& radius = 0.5f, const GLint& division = 18);

    static Model* CreateTerrain(const GLfloat& size, const GLint& vnum, const std::vector<GLfloat>& heightmap);
};