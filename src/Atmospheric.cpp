#include "app.hpp"

int main(int argc, char *argv[])
{
    Application* app = new Application();
    
    app->Init();
    app->Run();
    app->Cleanup();

    return 0;
}

