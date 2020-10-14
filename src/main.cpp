
#include "Common.hpp"

//Image loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//Physics
#include "BulletMain.h"
#include "terrain.hpp"
#include "cube.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "game.hpp"

GLFWwindow* window = NULL;
btDiscreteDynamicsWorld* world = NULL;
Game* game = NULL;


static void LoadTextures() 
{
    std::vector<std::string> paths = {
        "./resources/beach.png",
        "./resources/starnight.jpg",
        "./resources/grass.png",
        "./resources/brick.jpg",
        "./resources/metal.jpg"
    };
    for (int i = 0; i < paths.size(); i++) 
    {
        GLuint tex;
        glGenTextures(1, &tex);

        int width, height, nChannels;
        unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &nChannels, 0);
        if (data) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            std::cout << "Failed to load image " << i << std::endl;
        }
        stbi_image_free(data);
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
            ImGui::Begin("General Graphics");
            
            ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
            ImGui::Text("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
            ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
            ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
            
            GLint depth, stencil;
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);    
            glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);    
            ImGui::Text("Depth bits: %d", depth);
            ImGui::Text("Stencil bits: %d", stencil);

            GLint maxVertUniforms, maxFragUniforms;
            glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertUniforms);
            glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragUniforms);
            ImGui::Text("Max vertex uniforms: %d bytes", maxVertUniforms / 4);
            ImGui::Text("Max fragment uniforms: %d bytes", maxFragUniforms / 4);

            GLint maxVertUniBlocks, maxFragUniBlocks;
            glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertUniBlocks);
            glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragUniBlocks);
            ImGui::Text("Max vertex uniform blocks: %d", maxVertUniBlocks);
            ImGui::Text("Max fragment uniform blocks: %d", maxFragUniBlocks);

            ImGui::ColorEdit3("Clear color", (float*)&clearColor);
            ImGui::Text("Draw time: %.3f s/frame", dt);
            ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::Checkbox("Options", &show_another_window);
            ImGui::End();
        }

        // Show example windows
        /*
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
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
        */

        // Rendering
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);   

        game->Render();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);

        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR) {
            std::string error;
            switch (errorCode)
            {
                // Reference: https://learnopengl.com/In-Practice/Debugging
                case GL_INVALID_ENUM:                  
                error = "INVALID_ENUM"; break;
                case GL_INVALID_VALUE:                 
                error = "INVALID_VALUE"; break;
                case GL_INVALID_OPERATION:             
                error = "INVALID_OPERATION"; break;
                case GL_STACK_OVERFLOW:                
                error = "STACK_OVERFLOW"; break;
                case GL_STACK_UNDERFLOW:               
                error = "STACK_UNDERFLOW"; break;
                case GL_OUT_OF_MEMORY:                 
                error = "OUT_OF_MEMORY"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: 
                error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            }
            std::cout << "OpenGL Error " << error << std::endl;
        }
    }
    glfwSwapBuffers(window);
    glfwTerminate();
}

void OnError(int errorCode, const char* msg) {
    std::cout << "GLFW Error " << msg << std::endl;
}

int main(int argc, char *argv[]){
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    
    //Setup GL Framework
    glfwSetErrorCallback(OnError);
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 410";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
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
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    LoadTextures();    
    InitPhysics();

    game = new Game(window, world);
    game->Init(); // Construct the scene

    MainLoop();    
    return 0;
}

