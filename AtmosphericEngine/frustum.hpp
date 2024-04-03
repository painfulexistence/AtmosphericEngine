#pragma once
#include "globals.hpp"

struct Plane
{
    enum Halfspace {
        NEGATIVE = -1,
        ZERO = 0,
        POSITIVE = 1
    };

    float a, b, c, d;

    float SignedDistance(glm::vec3 p)
    {
        float naiveDist = a *  p.x + b * p.y + c * p.z + d;
        return naiveDist / glm::length(glm::vec3(a, b, c));
    };

    Halfspace Halfspace(glm::vec3 p)
    {
        float naiveDist = a *  p.x + b * p.y + c * p.z + d;
        if (naiveDist == 0)
            return ZERO;
        return naiveDist > 0 ? POSITIVE : NEGATIVE;
    };
};

class Frustum
{
    Plane _near, _far, _top, _bottom, _left, _right;

public:
    Frustum(glm::mat4 viewingMatrix);

    bool Intersects(glm::vec3);

    bool Intersects(std::array<glm::vec3, 8>);
};