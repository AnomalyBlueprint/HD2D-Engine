#pragma once
#include "Services/IInputService.h"
#include <vector>
#include <cstring> // for memcpy

/// <summary>
/// SVG input implementation using SDL2.
/// </summary>
class InputService : public IInputService
{
public:
    InputService();
    ~InputService();

    void Init() override;
    void Update() override;
    void Clean() override;

    bool IsKeyDown(SDL_Scancode key) override;
    bool IsKeyPressed(SDL_Scancode key) override;
    void GetMousePosition(int& x, int& y) override;
    int GetMouseScroll() override;
    
    /// <summary>
    /// Processes SDL events (e.g. Mouse Wheel).
    /// </summary>
    void OnEvent(const SDL_Event& e);

private:
    Uint8 m_currentKeyboardState[SDL_NUM_SCANCODES];
    Uint8 m_previousKeyboardState[SDL_NUM_SCANCODES];
    
    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_scrollDelta = 0;
};
