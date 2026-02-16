#pragma once
#include <SDL.h>
#include <memory>
#include <string>
#include "Core/Camera.h"
#include "Core/Player.h"
#include "Services/ChunkManager.h"

/// <summary>
/// Main Game Class.
/// Manages the Engine Lifecycle: Initialization, Game Loop, and Cleanup.
/// </summary>
class Game
{
public:
    Game() {}
    ~Game() {}

    /// <summary>
    /// Initializes SDL, OpenGL, and all Engine Services.
    /// </summary>
    void Init();

    /// <summary>
    /// Starts the main game loop.
    /// </summary>
    void Run();

    /// <summary>
    /// Shuts down the engine and releases resources.
    /// </summary>
    void Clean();

private:
    bool isRunning = false;
    SDL_Window *window = nullptr;
    std::shared_ptr<Camera> camera;
    
    // Shader IDs
    unsigned int basicShaderID = 0;
    unsigned int postEdgeShaderID = 0;
    unsigned int textureID = 0;
    
    std::unique_ptr<Player> m_player;

    Uint32 lastTime = 0; // Tracks the last frame's time
    float deltaTime = 0.0f; // Seconds per frame

    enum class RenderStyle { MINECRAFT, BORDERLANDS, MOCO };
    RenderStyle m_currentStyle = RenderStyle::BORDERLANDS;
};