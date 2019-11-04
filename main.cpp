#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>
#include <stdlib.h>
#include <time.h>
#include <array>
#include <list>
#include <thread>
#include <iostream>
#include "stb_image.h"
#include "terrain.hpp"
#include "cube.hpp"
#include "camera.hpp"

//#define DEBUG 0

const float SCREEN_W = 800.0f;
const float SCREEN_H = 600.0f;
const int MAZE_SIZE = 20;
const int TILES_TO_REMOVE = 150;
GLFWwindow* window;
btDiscreteDynamicsWorld* dynamicsWorld;

int initGL() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    window = glfwCreateWindow(SCREEN_W, SCREEN_H, "Maze", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();
    return 0;
}

void loadGlobalTextures() {
    std::array<std::string, 7> textures = {
      "textures/grass.png",
      "textures/starnight.jpg",
      "textures/beach.png",
      "textures/brick.jpg",
      "textures/skybox/bottom.bmp",
      "textures/skybox/top.bmp",
      "textures/skybox/front.bmp",
    };
    for (int i = 0; i < textures.size(); i++) {
      GLuint tex;
      glGenTextures(1, &tex);
      int width, height, nChannels;
      unsigned char *data = stbi_load(textures[i].c_str(), &width, &height, &nChannels, 0);
      if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
      } else {
        std::cout << "Failed to load image!" << std::endl;
      }
      stbi_image_free(data);
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, tex);
    }
}

void initPhysics() {
    btDefaultCollisionConfiguration *collisionCfg = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionCfg);
    btAxisSweep3 *axisSweep = new btAxisSweep3(btVector3(-100,-100,-100), btVector3(100,100,100), 16384); //will limit maximum number of collidable objects
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, axisSweep, solver, collisionCfg);
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
}

bool** generateMazeData(int size) {
    int mazeX = 1;
    int mazeY = 1;

    bool** data = new bool*[size];

    for (int i = 0; i < size; i++) {
        data[i] = new bool[size];
        for (int j = 0; j < size; j++) {
            data[i][j] = true;
        }
    }

    int tilesConsumed = 0;
    while (tilesConsumed < TILES_TO_REMOVE && tilesConsumed < (size - 2) * (size - 2)) {
        int xDir = 0;
        int yDir = 0;
        if (rand() % 2 < 0.5) {
            xDir = rand() % 2 < 0.5 ? 1 : -1;
        } else {
            yDir = rand() % 2 < 0.5 ? 1 : -1;
        }
        
        int moves = rand() % (size - 1) + 1;
        for (int i = 0; i < moves; i++) {
            mazeX = std::max(1, std::min(mazeX + xDir, size - 2));
            mazeY = std::max(1, std::min(mazeY + yDir, size - 2));
            if (data[mazeX][mazeY]) {
                data[mazeX][mazeY] = false;
                tilesConsumed++;
            }
        }
    }
    return data;
}

void createMaze(bool** maze, std::list<Cube*>& mazeBlocks, Camera* camera, int b_size = 1, bool hasRoof = false) {
    bool characterPlaced = false;

    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int z = 0; z < MAZE_SIZE; z++) {
            if (maze[x][z]) {
                mazeBlocks.push_back(new Cube(b_size, (float)b_size * glm::vec3(x - MAZE_SIZE / 2.f, 0, z - MAZE_SIZE / 2.f), 2, dynamicsWorld, 0.f));
                mazeBlocks.push_back(new Cube(b_size, (float)b_size * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f), 2, dynamicsWorld, 0.f));
                mazeBlocks.push_back(new Cube(b_size, (float)b_size * glm::vec3(x - MAZE_SIZE / 2.f, 2, z - MAZE_SIZE / 2.f), 2, dynamicsWorld, 0.f));
            } else {
                if (!characterPlaced) {
                    camera->setPosition((float)b_size * glm::vec3(x - MAZE_SIZE / 2.f, 1, z - MAZE_SIZE / 2.f));
                    characterPlaced = true;
                }
            }
            if (hasRoof) {
                mazeBlocks.push_back(new Cube(b_size, (float)b_size * glm::vec3(x - MAZE_SIZE / 2.f, 3, z - MAZE_SIZE / 2.f), 2, dynamicsWorld, 0.f));
            }

        }
    }
    #ifdef DEBUG
        printf("Maze created successfully!\n");
    #endif
}

int main() {
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created

    setbuf(stdout, NULL);

    if (initGL() < 0)
        return -1;

    loadGlobalTextures();

    initPhysics();

    /*
        Create camera
    */
    Camera* camera = new Camera(glm::vec3(0, 2, 0), 0.f, 0.f, glm::radians(45.0f), SCREEN_W / SCREEN_H, dynamicsWorld);
    Camera* camera2D = new Camera(glm::vec3(SCREEN_W / 2, SCREEN_H / 2, 0), 0.f, 0.f);

    /*
        Set property & create object
    */
    int cubeTexIndex = 2;
    std::list<Cube*> mazeBlocks;
    createMaze(generateMazeData(MAZE_SIZE), mazeBlocks, camera, 3, false);

    int terrainDrawMode = GL_FILL;
    float hills[100] = {
      1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
      2.0f, 3.0f, 6.0f, 8.0f, 5.0f, 3.0f, 5.0f, 8.0f, 10.0f, 6.0f,
      5.0f, 7.0f, 5.0f, 18.0f, 30.0f, 50.0f, 7.0f, 5.0f, 18.0f, 10.0f,
      20.0f, 23.0f, 6.0f, 20.0f, 50.0f, 2.0f, 3.0f, 6.0f, 8.0f, 5.0f,
      30.0f, 5.0f, 8.0f, 30.0f, 60.0f, 80.0f, 100.0f, 6.0f, 8.0f, 5.0f,
      52.0f, 7.0f, 5.0f, 18.0f, 10.0f, 50.0f, 100.0f, 6.0f, 8.0f, 5.0f,
      75.0f, 52.0f, 30.0f, 12.0f, 10.0f, 30.0f, 2.0f, -3.0f, 0.0f, 5.0f,
      20.0f, 30.0f, 6.0f, 8.0f, 5.0f, 3.0f, -5.0f, -18.0f, -10.0f, 1.0f,
      50.0f, 70.0f, 5.0f, 18.0f, 10.0f, 5.0f, -17.0f, -5.0f, 18.0f, 20.0f,
      20.0f, 30.0f, 6.0f, 8.0f, 5.0f, 1.0f, 2.0f, 30.0f, 40.0f, 50.0f
    };
    float plane[100] = {0};
    Cube* skybox = new Cube(800, glm::vec3(0, 0, 0), 0);
    Terrain* t = new Terrain(256, 10, plane, dynamicsWorld);
    Cube* box1 = new Cube(5, glm::vec3(2.5, 100, 2.5), 3, dynamicsWorld, 50.f);
    Cube* box2 = new Cube(3, glm::vec3(4, 120, 4), 3, dynamicsWorld, 30.f);
    Cube* box3 = new Cube(1, glm::vec3(2, 180, 3), 3, dynamicsWorld, 10.f);

    /*
        Set timer
    */
    double pastTime = glfwGetTime();
    double timeAccumulator = 0;
    const double timeDelta = 1.f / 60.f;

    /*
        Set up renderer
    */
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    while(!glfwWindowShouldClose(window))
    {
        /*
            Step physics simulation
        */
        #ifdef DEBUG
            printf("Before simulation steps...\n");
        #endif
        while (timeAccumulator >= timeDelta) {
            if(dynamicsWorld != NULL) {
                dynamicsWorld->stepSimulation(timeDelta, 120);
            }
            timeAccumulator -= timeDelta;
        }    
        #ifdef DEBUG
            printf("Simulation steps successfully!\n");
        #endif

        /*
            Rendering
        */
        //clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        t->render(camera->getProjectionViewMatrix() * glm::mat4(1.0f), terrainDrawMode);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)(glfwGetTime() * glm::radians(10.0f)), glm::vec3(0, 1, 0));
        skybox->render(camera->getProjectionViewMatrix() * model);

        for (Cube* cube : mazeBlocks) {
            cube->render(camera->getProjectionViewMatrix() * cube->getDynamicsTransform());
        }

        box1->render(camera->getProjectionViewMatrix() * box1->getDynamicsTransform());
        box2->render(camera->getProjectionViewMatrix() * box2->getDynamicsTransform());
        box3->render(camera->getProjectionViewMatrix() * box3->getDynamicsTransform());

        glfwSwapBuffers(window);

        /*
            Process events
        */
        glfwPollEvents();
        double currentTime = glfwGetTime();
        float dt = float(currentTime - pastTime);
        pastTime = currentTime;
        timeAccumulator += dt;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        
        camera->damp();
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            camera->backAndForthMove(1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            camera->backAndForthMove(-1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            camera->yaw(1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            camera->yaw(-1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->yaw(1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->yaw(-1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->pitch(1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->pitch(-1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera->verticallyMove(1.0, dt);
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            terrainDrawMode = terrainDrawMode == GL_FILL ? GL_LINE : GL_FILL;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            cubeTexIndex = cubeTexIndex >= 5 ? 0 : cubeTexIndex + 1;
            for (Cube* cube : mazeBlocks) {
                cube->setTexture(cubeTexIndex);
            }
        }
    }
    glfwTerminate();
}
