#include "app.hpp"

Application::Application() : framework(new Framework()) {}

Application::~Application() {}

void Application::Init()
{
    //setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    framework->Init();
}

void Application::Run() 
{
    Game game(framework);
    game.Init();
    game.Start();
    
    double pastTime = framework->GetTime();
    while(!framework->ShouldCloseWindow())
    {
        double currentTime = framework->GetTime();
        float dt = float(currentTime - pastTime);
        pastTime = currentTime;

        // Start game frame
        game.Update(dt);

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
    framework->Terminate();
}
