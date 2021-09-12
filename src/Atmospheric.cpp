#include "game.hpp"

int main(int argc, char* argv[])
{
    //setbuf(stdout, NULL); //Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL)); //Dont's use glfwGetTime() bc it only starts to calculate time since window was created

    Framework framework = Framework();
    Renderer renderer = Renderer();
    renderer.Configure();
    renderer.CreateGUI(framework);
    renderer.CreateBuffers();

    Game game(framework, renderer);
    game.Create();
    
    double pastTime = framework.GetTime();
    while(!framework.IsWindowOpen())
    {
        double currentTime = framework.GetTime();
        float deltaTime = (float)currentTime - (float)pastTime;
        pastTime = currentTime;

        game.Update(deltaTime, (float)currentTime);
        double ut = framework.GetTime();

        game.Render(deltaTime, (float)currentTime);
        double rt = framework.GetTime();
        //std::cout << "Update cost: " << (float)ut - (float)currentTime << "secs, " << "render cost: " << (float)rt - (float)ut << "secs" << std::endl;
    }  
    return 0;
}

