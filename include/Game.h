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
    unsigned int basicShaderID = 0;
    unsigned int textureID = 0;
    
    // Voxel Chunk Data
    unsigned int chunkMeshID = 0;
    int chunkIndexCount = 0;

    // Debug Cube Data
    // Debug Cube Removed
    // unsigned int debugMeshID = 0;
    // int debugIndexCount = 0;
    
    Uint32 lastTime = 0; // Tracks the last frame's time
    float deltaTime = 0.0f; // Seconds per frame
};