#pragma once
#include "Services/IService.h"
#include <SDL.h>

/// <summary>
/// Interface for input handling (Keyboard/Mouse).
/// </summary>
class IInputService : public IService {
public:
    virtual ~IInputService() = default;
    
    /// <summary>
    /// Updates input state. Call at the start of every frame.
    /// </summary>
    virtual void Update() = 0;
    
    /// <summary>
    /// Checks if a key is currently held down.
    /// </summary>
    /// <param name="key">The SDL Scancode to check.</param>
    /// <returns>True if held down.</returns>
    virtual bool IsKeyDown(SDL_Scancode key) = 0;
    
    /// <summary>
    /// Checks if a key was just pressed this frame.
    /// </summary>
    /// <param name="key">The SDL Scancode to check.</param>
    /// <returns>True if pressed this frame.</returns>
    virtual bool IsKeyPressed(SDL_Scancode key) = 0;

    /// <summary>
    /// Gets the current mouse position relative to the window.
    /// </summary>
    /// <param name="x">Output X position.</param>
    /// <param name="y">Output Y position.</param>
    virtual void GetMousePosition(int& x, int& y) = 0;

    /// <summary>
    /// Gets the mouse scroll delta for this frame.
    /// </summary>
    /// <returns>Positive for up, negative for down.</returns>
    virtual int GetMouseScroll() = 0;
};