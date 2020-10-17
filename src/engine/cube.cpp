#include "cube.hpp"

Cube::Cube(int size)
{
    _size = size;

    numVert = 192;
    vertices = new float[numVert] {
        //left
        .5f * _size, .5f * _size, .5f * _size, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        .5f * _size, -.5f * _size, .5f * _size, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -.5f * _size, .5f * _size, .5f * _size, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -.5f * _size, -.5f * _size, .5f * _size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        //right
        .5f * _size, .5f * _size, -.5f * _size, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        .5f * _size, -.5f * _size, -.5f * _size, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -.5f * _size, .5f * _size, -.5f * _size, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -.5f * _size, -.5f * _size, -.5f * _size, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        //back
        -.5f * _size, .5f * _size, .5f * _size, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -.5f * _size, .5f * _size, -.5f * _size, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -.5f * _size, -.5f * _size, .5f * _size, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -.5f * _size, -.5f * _size, -.5f * _size, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //front
        .5f * _size, .5f * _size, .5f * _size, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        .5f * _size, .5f * _size, -.5f * _size, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        .5f * _size, -.5f * _size, .5f * _size, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        .5f * _size, -.5f * _size, -.5f * _size, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //top
        .5f * _size, .5f * _size, .5f * _size, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        .5f * _size, .5f * _size, -.5f * _size, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -.5f * _size, .5f * _size, .5f * _size, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -.5f * _size, .5f * _size, -.5f * _size, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        //bottom
        .5f * _size, -.5f * _size, .5f * _size, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        .5f * _size, -.5f * _size, -.5f * _size, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -.5f * _size, -.5f * _size, .5f * _size, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -.5f * _size, -.5f * _size, -.5f * _size, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };

    numTri = 36;
    triangles = new GLushort[numTri] {
        0, 2, 1,
        1, 2, 3,
        4, 5, 6,
        6, 5, 7,
        8, 9, 10,
        10, 9, 11,
        12, 14, 13,
        13, 14, 15,
        16, 17, 18,
        18, 17, 19,
        20, 22, 21,
        21, 22, 23
    };
}

void Cube::Embody(glm::vec3 center, float mass = 0.f, btDiscreteDynamicsWorld* world = nullptr)
{
    if (world == 0)
    {
        std::runtime_error("Failed to create rigidbody");
        return;
    }
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(center.x, center.y, center.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, _size / 2.f, _size / 2.f));

    rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    world->addRigidBody(rigidbody);
}

void Cube::Render(std::vector<glm::mat4> worldMatrices)
{
    if (!initialized)
        std::runtime_error("VAO uninitialized!");
    
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<int>(worldMatrices.size()) * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, numTri, GL_UNSIGNED_SHORT, 0, static_cast<int>(worldMatrices.size()));
    glBindVertexArray(0);
}

void Cube::Render(glm::mat4* worldMatrices, int num)
{
    if (!initialized)
        std::runtime_error("VAO uninitialized!");
    
    glBindVertexArray(vao);
    
    // Fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVert*sizeof(float), vertices, GL_STATIC_DRAW);
    
    // Fill element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTri*sizeof(GLushort), triangles, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(glm::mat4), worldMatrices, GL_STATIC_DRAW);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, numTri, GL_UNSIGNED_SHORT, 0, num);
    glBindVertexArray(0);
}
