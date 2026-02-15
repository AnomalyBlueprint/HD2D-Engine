#pragma once
#include "Services/RenderService.h"
#include <SDL.h>
#include <GL/glew.h>

class OpenGLRenderService : public RenderService
{
public:
    OpenGLRenderService(SDL_Window *window);
    ~OpenGLRenderService();

    void Init() override;
    void Clean() override;
    void Clear() override;
    void SwapBuffers() override;

private:
    SDL_Window *window;
    SDL_GLContext context;
};