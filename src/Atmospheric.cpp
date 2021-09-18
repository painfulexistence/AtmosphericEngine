#include "MazeGame.hpp"

int main(int argc, char* argv[])
{
    //setbuf(stdout, NULL); // Cancel output stream buffering so that output can be seen immediately
    srand(time(NULL));

    MazeGame game;
    game.Execute();
    
    return 0;
}

