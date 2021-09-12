#pragma once
#include "../common.hpp"
#include "plane.hpp"

class Frustum
{
    Plane _near, _far, _top, _bottom, _left, _right;
    
public:
    Frustum(glm::mat4 viewingMatrix);

    bool Intersects(glm::vec3);

    bool Intersects(std::array<glm::vec3, 8>);
};