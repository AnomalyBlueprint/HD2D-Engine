#include "Services/InputService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>

InputService::InputService()
{
    // Initialize arrays to 0
    std::memset(m_currentKeyboardState, 0, SDL_NUM_SCANCODES);
    std::memset(m_previousKeyboardState, 0, SDL_NUM_SCANCODES);
}

InputService::~InputService()
{
    Clean();
}

void InputService::Init()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    log->Log("Input Service Initialized.");
}

void InputService::Update()
{
    // Reset per-frame inputs
    m_scrollDelta = 0;

    // Update Previous Frame before fetching new state
    std::memcpy(m_previousKeyboardState, m_currentKeyboardState, SDL_NUM_SCANCODES);

    // Get current state from SDL
    const Uint8* state = SDL_GetKeyboardState(NULL);
    std::memcpy(m_currentKeyboardState, state, SDL_NUM_SCANCODES);

    // Update Mouse
    SDL_GetMouseState(&m_mouseX, &m_mouseY);
}

void InputService::OnEvent(const SDL_Event& e)
{
    if (e.type == SDL_MOUSEWHEEL)
    {
        m_scrollDelta = e.wheel.y;
    }
}

void InputService::Clean()
{
    // Nothing specific to clean for input
}

bool InputService::IsKeyDown(SDL_Scancode key)
{
    // 1 means pressed, 0 means released
    return m_currentKeyboardState[key] == 1;
}

bool InputService::IsKeyPressed(SDL_Scancode key)
{
    // It's a "press" if it is DOWN now, but was UP last frame
    return (m_currentKeyboardState[key] == 1) && (m_previousKeyboardState[key] == 0);
}

void InputService::GetMousePosition(int& x, int& y)
{
    x = m_mouseX;
    y = m_mouseY;
}

int InputService::GetMouseScroll()
{
    return m_scrollDelta;
}
