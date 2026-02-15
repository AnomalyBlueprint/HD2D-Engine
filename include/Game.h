#pragma once
#include <SDL.h>
#include <memory>
#include <string>
#include "Camera.h"

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
    std::shared_ptr<Camera> camera;
    unsigned int triangleMeshID = 0;
    unsigned int basicShaderID = 0;
    unsigned int textureID = 0;
};