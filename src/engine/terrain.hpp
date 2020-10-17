#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"
#include "geometry.hpp"

class Terrain : public Geometry
{
private:
    int _size;

public:
    Terrain(int, int, float[]);

    void Embody(glm::vec3, float, btDiscreteDynamicsWorld*) override;

    void Render(std::vector<glm::mat4>) override;
    
    void Render(glm::mat4*, int) override;
};
