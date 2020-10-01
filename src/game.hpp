#pragma once
#include "globals.h"
#include <btBulletDynamicsCommon.h>
#include <list>
#include "shader.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "cube.hpp"
#include "terrain.hpp"

class Game
{
    private:
    GLFWwindow* _window;
    Shader* _shader;
    btDiscreteDynamicsWorld* _world;
    Camera* camera;
    Light* light;
    Cube* skybox;
    Terrain* terrain;
    std::list<Mesh*> mazeBlocks;

    glm::vec3 lightColor = glm::vec3(1.0, 0.5, 0.25);
    bool isLightFlashing = false;
    int terrainDrawMode = GL_LINE;
    glm::vec3 winPosition;

    double timeAccumulator;
    const double timeDelta = 1.f / 60.f;
        
    public:
    Game(GLFWwindow* window, Shader* shader, btDiscreteDynamicsWorld* world);
    ~Game();

    void Init();
    int Update(float dt);
    void Render();
};