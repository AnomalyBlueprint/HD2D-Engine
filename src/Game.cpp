#include "Game.h"
#include "Services/ServiceLocator.h"
#include "Services/RenderService.h"
#include "Services/OpenGLRenderService.h"
#include "Services/ILoggerService.h"
#include "Services/LogTypes.h"
#include "Services/LoggerService.h"
#include <iostream>

void Game::Init()
{
    auto logger = std::make_shared<LoggerService>();
    ServiceLocator::Get().Register<ILoggerService>(logger);

    auto log = ServiceLocator::Get().GetService<ILoggerService>();

    log->Log("Engine Starting...");        // Defaults to Info
    log->LogWarning("This is a warning!"); // Yellow
    log->LogError("This is an error!");    // Red

    // 1. Init SDL Window
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        log->LogError("Failed to initialize SDL: " + std::string(SDL_GetError()));
        return;
    }

    // OpenGL Attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("HD-2D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // 2. BOOTSTRAP SERVICES
    // Register the OpenGL Renderer as the provider for "RenderService"
    auto renderer = std::make_shared<OpenGLRenderService>(window);
    ServiceLocator::Get().Register<RenderService>(renderer);

    isRunning = true;
    log->Log("Engine Started!");
}

void Game::Run()
{
    while (isRunning)
    {
        // 1. Input
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                isRunning = false;
        }

        // 2. Update
        // ServiceLocator::Get().GetService<UpdateService>()->Update();

        // 3. Render (Using the Service!)
        auto renderer = ServiceLocator::Get().GetService<RenderService>();
        if (renderer)
        {
            renderer->Clear();
            // renderer->Draw(...);
            renderer->SwapBuffers();
        }
    }
}

void Game::Clean()
{
    ServiceLocator::Get().Clean();
    SDL_DestroyWindow(window);
    SDL_Quit();
}