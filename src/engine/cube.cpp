#include "cube.hpp"

Cube::Cube(int size)
{
    _size = size;

    GLfloat vertices[] = {
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
    verts.assign(vertices, vertices + 192);

    GLushort triangles[] = {
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
    tris.assign(triangles, triangles + 36);
}

void Cube::Embody(glm::vec3 center, float mass = 0.f, const std::shared_ptr<btDiscreteDynamicsWorld>& world = nullptr)
{
    if (world == 0)
    {
        throw std::runtime_error("Failed to create rigidbody");
    }
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(center.x, center.y, center.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, _size / 2.f, _size / 2.f));

    rigidbody = std::make_shared<btRigidBody>(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    world->addRigidBody(rigidbody.get());
}
