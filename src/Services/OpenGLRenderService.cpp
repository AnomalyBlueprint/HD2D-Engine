#include "Services/OpenGLRenderService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>

OpenGLRenderService::OpenGLRenderService(SDL_Window *win) : window(win) {}

OpenGLRenderService::~OpenGLRenderService()
{
    Clean();
}

void OpenGLRenderService::Init()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    // 1. Create Context
    context = SDL_GL_CreateContext(window);
    if (!context)
    {
        log->LogError("Failed to create OpenGL context: " + std::string(SDL_GetError()));
        return;
    }

    // 2. Init GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        log->LogError("Failed to initialize GLEW: " + std::string((const char *)glewGetErrorString(err)));
        return;
    }

    // 3. Setup
    glViewport(0, 0, 1280, 720);
    glEnable(GL_DEPTH_TEST);

    log->Log("OpenGL Renderer Initialized.");

}

void OpenGLRenderService::Clear()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderService::SwapBuffers()
{
    SDL_GL_SwapWindow(window);
}

void OpenGLRenderService::Clean()
{
    if (context != nullptr)
    {
        SDL_GL_DeleteContext(context);
        context = nullptr;
    }
}