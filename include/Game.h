#pragma once
#include <SDL.h>
#include <memory>
#include <string>
#include "Core/Camera.h"
#include "Core/Player.h"
#include "Services/ChunkManager.h"

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
    unsigned int postEdgeShaderID = 0;
    unsigned int textureID = 0;
    
    std::unique_ptr<Player> m_player;

    // Debug Cube Data
    // Debug Cube Removed
    // unsigned int debugMeshID = 0;
    // int debugIndexCount = 0;
    
    Uint32 lastTime = 0; // Tracks the last frame's time
    float deltaTime = 0.0f; // Seconds per frame

    enum class RenderStyle { MINECRAFT, BORDERLANDS, MOCO };
    RenderStyle m_currentStyle = RenderStyle::BORDERLANDS;
};