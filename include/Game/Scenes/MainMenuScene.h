#pragma once
#include "Engine/Scene/IScene.h"
#include <SDL.h>
#include <glm/glm.hpp>

class MainMenuScene : public IScene {
public:
    MainMenuScene();
    ~MainMenuScene() override;

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnEvent(SDL_Event& e) override;

private:
    void RenderMacroMap();
    void UpdateMapView();

    // Map View State
    float m_mapZoom = 1.0f;
    glm::vec2 m_mapOffset = glm::vec2(0.0f);
    
    unsigned int m_basicShaderID = 0;

    // UI Logic
    bool m_debugOverlay = false;
};
