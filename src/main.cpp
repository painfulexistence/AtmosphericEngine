
#include "globals.h"

//Image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//Physics
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <LinearMath/btTransform.h>

#include <stdlib.h>
#include <time.h>
#include <array>
#include <list>
#include <thread>
#include <iostream>

#include "shader.hpp"
#include "mesh.hpp"
#include "terrain.hpp"
#include "cube.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "game.hpp"

#define TEXTURES_COUNT 12

GLFWwindow* window = NULL;
GLuint gVAO = 0;
GLuint gVBO = 0;

btDiscreteDynamicsWorld* world = NULL;
Shader* shader = NULL;
Game* game = NULL;

void LoadShaders()
{
    shader = new Shader();
    shader->Load();
    shader->Attach(0);
    shader->Attach(1);
    shader->Activate();
}

void LoadTextures() 
{
    GLuint textures[TEXTURES_COUNT];
    glGenTextures(TEXTURES_COUNT, textures);
    std::array<std::string, TEXTURES_COUNT> texture_paths = {
        "resources/grass.png",
        "resources/starnight.jpg",
        "resources/beach.png",
        "resources/brick.jpg",
        "resources/metal.jpg",
        "resources/grass.png",
        "resources/grass.png",
        "resources/grass.png",
        "resources/grass.png",
        "resources/grass.png",
        "resources/grass.png",
        "resources/grass.png"
    };
    for (int i = 0; i < TEXTURES_COUNT; i++) {
        int width, height, nChannels;
        unsigned char *data = stbi_load(texture_paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load image!" << std::endl;
        }
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        stbi_image_free(data);
        glUniform1i(shader->GetUniform(("tex_" + std::to_string(i+1)).c_str()), i);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void InitPhysics() {
    btDefaultCollisionConfiguration *collisionCfg = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionCfg);
    btAxisSweep3 *axisSweep = new btAxisSweep3(btVector3(-100,-100,-100), btVector3(100,100,100), 16384); //will limit maximum number of collidable objects
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;
    world = new btDiscreteDynamicsWorld(dispatcher, axisSweep, solver, collisionCfg);
    world->setGravity(btVector3(0, -10, 0));
}

void MainLoop() {
    // Application state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    double pastTime = glfwGetTime();

    while(!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        float dt = float(currentTime - pastTime);
        pastTime = currentTime;

        // Start the game frame
        if (game->Update(dt) < 0) break;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show a custom window
        {
            ImGui::Begin("Graphics");
            ImGui::ColorEdit3("Clear color", (float*)&clearColor);
            ImGui::Text("Draw time %.3f s/frame, frame rate %.1f FPS", dt, 1.0f / dt);
            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::Checkbox("Options", &show_another_window);
            ImGui::End();
        }

        // Show other example windows
        /*
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        */
        if (show_another_window)
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Test Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::Button("Button"))
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        game->Render();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwTerminate();
}

int main(int argc, char *argv[]){
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    
    //Setup GL Framework
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "Atmospheric Engine", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Setup OpenGL extension loader
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return 1;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFF);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    LoadShaders();
    LoadTextures();
    InitPhysics();

    game = new Game(window, shader, world);
    game->Init();

    MainLoop();    
    return 0;
}

