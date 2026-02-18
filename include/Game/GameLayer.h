#pragma once
#include <SDL.h>
#include <memory>

class GameLayer {
public:
    GameLayer();
    virtual ~GameLayer();

    virtual void OnAttach();
    virtual void OnDetach();
    virtual void OnUpdate(float deltaTime);
    virtual void OnRender();
    virtual void OnEvent(SDL_Event& e);
};
