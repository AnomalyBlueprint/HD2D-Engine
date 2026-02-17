#pragma once
#include <SDL.h>
#include <memory>
#include <vector>
#include "Game/GameLayer.h"

/// <summary>
/// Engine Core Class.
/// Handles Initialization, Common Services, and the Main Loop.
/// </summary>
class Engine
{
public:
    Engine();
    ~Engine();

    void Init();
    void Run();
    void Clean();

    void AttachLayer(std::shared_ptr<GameLayer> layer);
    
    void ToggleFullscreen();

private:
    bool m_isRunning = false;
    bool m_fullscreen = false;
    SDL_Window* m_window = nullptr;
    std::shared_ptr<GameLayer> m_gameLayer; // Currently single layer support

    Uint32 m_lastTime = 0;
};
