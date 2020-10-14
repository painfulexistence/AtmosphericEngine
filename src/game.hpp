#pragma once
#include "Common.hpp"
#include "BulletMain.h"
#include "material.hpp"
#include "entity.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "cube.hpp"
#include "terrain.hpp"

class Game
{
private:
    GLFWwindow* _window;
    btDiscreteDynamicsWorld* _world;
    std::vector<Material*> _materials;
    Camera* camera;
    Light* light;
    Cube* skybox;
    Terrain* terrain;
    std::vector<Cube*> cubes;

    glm::vec3 lightColor = glm::vec3(1, 1, 1);
    bool isLightFlashing = false;
    glm::vec3 winPosition;

    double timeAccumulator;
    const double timeDelta = 1.f / 60.f;
    
public:
    Game(GLFWwindow*, btDiscreteDynamicsWorld*);
    
    ~Game();

    void Init();

    int Update(float);

    void Render();
};