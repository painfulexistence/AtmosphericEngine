#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"
#include "geometry.hpp"

class Sphere : public Geometry 
{
    float _radius = 0.5f;

public:
    Sphere(float);

    void Embody(glm::vec3, float, const std::shared_ptr<btDiscreteDynamicsWorld>&) override;

    void Render(std::vector<glm::mat4>, GLenum) override;

};
