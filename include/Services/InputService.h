#pragma once
#include "Services/IInputService.h"
#include <vector>
#include <cstring> // for memcpy

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
    // Call this from Game Loop Event Polling
    void OnEvent(const SDL_Event& e);

private:
    // SDL_NUM_SCANCODES is usually 512
    Uint8 m_currentKeyboardState[SDL_NUM_SCANCODES];
    Uint8 m_previousKeyboardState[SDL_NUM_SCANCODES];
    
    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_scrollDelta = 0;
};
