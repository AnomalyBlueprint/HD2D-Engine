#include "Game.h"

int main(int argc, char *argv[])
{
    Game *game = new Game();

    game->Init();
    game->Run();
    game->Clean();

    delete game;
    return 0;
}