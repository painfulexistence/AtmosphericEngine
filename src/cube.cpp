#include "cube.hpp"

Cube::Cube(int size)
{
    _size = size;
    vertices = new float[192] {
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

    triangles = new GLushort[36] {
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(float), vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements*sizeof(GLushort), triangles, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(4 * sizeof(float)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(8 * sizeof(float)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(3); 
    glEnableVertexAttribArray(4); 
    glEnableVertexAttribArray(5); 
    glEnableVertexAttribArray(6); 
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

void Cube::AddRigidBody(glm::vec3 position, btDiscreteDynamicsWorld* dynamicsWorld, float mass = .0f)
{
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, _size / 2.f, _size / 2.f));

    _rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    dynamicsWorld->addRigidBody(_rigidbody);
}

void Cube::Render() 
{
    glBindVertexArray(vao);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0); 
    glBindVertexArray(0);
}

void Cube::RenderMultiple(glm::mat4* worldMatrices, int number)
{
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, number * sizeof(glm::mat4), worldMatrices, GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0, number); 
    glBindVertexArray(0);
}