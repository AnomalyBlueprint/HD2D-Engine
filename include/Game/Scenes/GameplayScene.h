#pragma once
#include "Engine/Scene/IScene.h"
#include "Engine/Core/Camera.h"
#include "Game/Player.h"
#include <SDL.h>
#include <memory>
#include "Engine/Services/RenderService.h"

class GameplayScene : public IScene {
public:
    GameplayScene();
    ~GameplayScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnEvent(SDL_Event& e) override;

private:
    std::shared_ptr<class Camera> m_camera;
    std::unique_ptr<Player> m_player;

    unsigned int m_basicShaderID = 0;
    unsigned int m_postEdgeShaderID = 0;
    unsigned int m_textureID = 0;
    
    // Render Styles (Local to scene or Global config? Keeping local for now as per GameLayer)
    enum class RenderStyle { MINECRAFT, BORDERLANDS, MOCO };
    RenderStyle m_currentStyle = RenderStyle::BORDERLANDS;
    
    int m_debugMode = 0;
};
