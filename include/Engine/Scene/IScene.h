#pragma once
#include <SDL.h>

class IScene {
public:
    virtual ~IScene() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRender() = 0;
    virtual void OnEvent(SDL_Event& e) = 0;
};
