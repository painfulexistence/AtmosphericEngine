#pragma once
#include "../common.hpp"

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