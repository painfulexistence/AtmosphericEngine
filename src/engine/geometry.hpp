#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"
#include "../physics/Debugger.hpp"
#include "material.hpp"
#include "program.hpp"


class Geometry 
{
protected:
    int _id;
    GLuint vao, vbo, ebo, ibo;
    std::vector<GLfloat> verts;
    std::vector<GLushort> tris;
    std::shared_ptr<btRigidBody> rigidbody = nullptr;
    bool initialized = false;

public:
    Geometry();

    ~Geometry();

    void Init();

    glm::mat4 GetTransform() 
    {
        if (rigidbody == 0)
            return glm::mat4(1.0f);
        
        btTransform t;
        rigidbody->getMotionState()->getWorldTransform(t);
        
        btScalar mat[16] = { 0.0f };
        t.getOpenGLMatrix(mat);
        
        glm::mat4 wm = glm::mat4(
            mat[0], mat[1], mat[2], mat[3],
            mat[4], mat[5], mat[6], mat[7],
            mat[8], mat[9], mat[10], mat[11],
            mat[12], mat[13], mat[14], mat[15]
        );
        return wm;
    };

    virtual void Embody(glm::vec3, float, const std::shared_ptr<btDiscreteDynamicsWorld>&);

    virtual void Update(float);

    virtual void Render(std::vector<glm::mat4>, GLenum = GL_TRIANGLES);
    
    virtual void RenderWithOutline(std::vector<glm::mat4>);
};
