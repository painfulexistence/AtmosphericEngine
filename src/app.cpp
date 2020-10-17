#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "app.hpp"

Application::Application()
{
    framework = new Framework();
    game = new Game(framework);
}

Application::~Application()
{
    delete game;
    delete framework;
}

void Application::Init()
{
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    
    framework->Init();
}

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

void Application::Run()
{
    LoadTextures();
    MainLoop();
}

void Application::Cleanup()
{
    framework->Terminate();
}

void Application::MainLoop() 
{
    game->Init();
    game->Start();
    
    pastTime = framework->GetTime();
    while(!framework->ShouldCloseWindow())
    {
        double currentTime = framework->GetTime();
        float dt = float(currentTime - pastTime);
        pastTime = currentTime;

        // Start game frame
        game->Update(dt);

        // Error detecting
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
}
