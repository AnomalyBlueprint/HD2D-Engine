#pragma once
#include "Engine/Core/IService.h"
#include "Engine/Scene/IScene.h"
#include <memory>
#include <string>

class SceneService : public IService {
public:
    SceneService() = default;
    ~SceneService() = default;

    void LoadScene(IScene* newScene); // Takes ownership

    void Update(float deltaTime);
    void Render();
    void OnEvent(SDL_Event& e);

    // IService Implementation
protected:
    void OnInitialize() override {}

public:
    void Clean() override;

private:
    std::unique_ptr<IScene> m_currentScene;
};
