#pragma once
#include "Globals.hpp"
#include "BulletCollision.hpp"

class Collider
{
public:
    Collider();

    ~Collider();

private:
    btCollisionShape* _collisionShape;
    glm::vec3 _positionOffset = glm::vec3(0, 0, 0);
    glm::vec3 _rotationOffset = glm::vec3(0, 0, 0);
};