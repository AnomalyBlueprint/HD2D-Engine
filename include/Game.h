#pragma once
#include <SDL.h>
#include <memory>
#include <string>

class Game
{
public:
    Game() {}
    ~Game() {}

    void Init();
    void Run();
    void Clean();

private:
    bool isRunning = false;
    SDL_Window *window = nullptr;
};