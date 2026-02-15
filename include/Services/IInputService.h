#pragma once
#include "Services/IService.h"
#include <SDL.h>

class IInputService : public IService {
public:
    virtual ~IInputService() = default;
    
    // Call this at the start of every frame
    virtual void Update() = 0;
    
    // Check if a key is currently held down
    virtual bool IsKeyDown(SDL_Scancode key) = 0;
    
    // Check if a key was just pressed this frame (for things like Jump or Menu)
    virtual bool IsKeyPressed(SDL_Scancode key) = 0;
};