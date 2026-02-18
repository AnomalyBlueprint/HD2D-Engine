#include "Engine/Services/Scene/SceneService.h"

void SceneService::LoadScene(IScene* newScene) {
    if (m_currentScene) {
        m_currentScene->OnExit();
    }

    m_currentScene.reset(newScene);

    if (m_currentScene) {
        m_currentScene->OnEnter();
    }
}

void SceneService::Update(float deltaTime) {
    if (m_currentScene) {
        m_currentScene->OnUpdate(deltaTime);
    }
}

void SceneService::Render() {
    if (m_currentScene) {
        m_currentScene->OnRender();
    }
}

void SceneService::OnEvent(SDL_Event& e) {
    if (m_currentScene) {
        m_currentScene->OnEvent(e);
    }
}

void SceneService::Clean() {
    if (m_currentScene) {
        m_currentScene->OnExit();
        m_currentScene.reset();
    }
}
