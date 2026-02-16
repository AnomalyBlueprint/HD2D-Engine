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

private:
    std::shared_ptr<Camera> m_camera;
    std::unique_ptr<Player> m_player;

    // Shader IDs
    unsigned int m_basicShaderID = 0;
    unsigned int m_postEdgeShaderID = 0;
    unsigned int m_textureID = 0;

    enum class RenderStyle { MINECRAFT, BORDERLANDS, MOCO };
    RenderStyle m_currentStyle = RenderStyle::BORDERLANDS;
};
