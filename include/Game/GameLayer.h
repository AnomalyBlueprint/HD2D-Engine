#pragma once
#include <SDL.h>
#include <memory>
#include "Engine/Core/Camera.h"
#include "Game/Player.h"

// Forward declarations to avoid circular includes if possible
class GameLayer {
public:
    GameLayer();
    virtual ~GameLayer();

    virtual void OnAttach();
    virtual void OnDetach();
    virtual void OnUpdate(float deltaTime);
    virtual void OnRender();
    virtual void OnEvent(SDL_Event& e);
    
    void RenderAtlasDebug(class RenderService* renderer);

private:
    std::shared_ptr<class Camera> m_camera;
    std::unique_ptr<Player> m_player;

    // Shader IDs
    unsigned int m_basicShaderID = 0;
    unsigned int m_postEdgeShaderID = 0;
    unsigned int m_textureID = 0;

    enum class RenderStyle { MINECRAFT, BORDERLANDS, MOCO };
    int m_debugMode = 0;
    
    RenderStyle m_currentStyle = RenderStyle::BORDERLANDS;

    enum class GameState { MainMenu, Gameplay, DebugOverlay };
    GameState m_currentState = GameState::MainMenu;
};
