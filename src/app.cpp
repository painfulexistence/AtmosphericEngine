#include "app.hpp"

Application::Application() : framework(Framework()) {}

void Application::Init()
{
    //setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created
    framework.Init();
}

void Application::Run() 
{
    Game game(framework);
    game.Run();
    
    framework.Terminate();
}
