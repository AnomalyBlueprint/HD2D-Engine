#include "Engine/Services/Input/InputService.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/ILoggerService.h"
#include <iostream>

InputService::InputService()
{
    std::memset(m_currentKeyboardState, 0, SDL_NUM_SCANCODES);
    std::memset(m_previousKeyboardState, 0, SDL_NUM_SCANCODES);
}

InputService::~InputService()
{
    Clean();
}

void InputService::OnInitialize()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    log->Log("Input Service Initialized.");
}

void InputService::Update()
{
    m_scrollDelta = 0;

    std::memcpy(m_previousKeyboardState, m_currentKeyboardState, SDL_NUM_SCANCODES);

    const Uint8* state = SDL_GetKeyboardState(NULL);
    std::memcpy(m_currentKeyboardState, state, SDL_NUM_SCANCODES);

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
}

bool InputService::IsKeyDown(SDL_Scancode key)
{
    return m_currentKeyboardState[key] == 1;
}

bool InputService::IsKeyPressed(SDL_Scancode key)
{
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
