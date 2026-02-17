#include "Engine/Engine.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/LoggerService.h"
#include "Engine/Services/OpenGLRenderService.h"
#include "Engine/Services/InputService.h"
#include "Engine/Services/ResourceService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/KenneyPathRepository.h"
#include "Engine/Services/KenneyPathRepository.h"
#include "Engine/Services/FontPathRepository.h"
#include "Engine/Services/OpenGLShaderService.h"
#include "Engine/Services/FontService.h"
#include <GL/glew.h>

Engine::Engine() {}

Engine::~Engine() {}

void Engine::Init()
{
    // 1. Logger
    auto loggerServ = std::make_shared<LoggerService>();
    ServiceLocator::Get().Register<ILoggerService>(loggerServ);
    ServiceLocator::Get().GetService<ILoggerService>()->Log("Engine Starting...");

    // 2. SDL Setup
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        ServiceLocator::Get().GetService<ILoggerService>()->LogError("SDL Init Failed!");
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    m_window = SDL_CreateWindow("HD-2D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    // 3. Core Services
    auto renderer = std::make_shared<OpenGLRenderService>(m_window);
    ServiceLocator::Get().Register<RenderService>(renderer);

    auto input = std::make_shared<InputService>();
    ServiceLocator::Get().Register<IInputService>(input);

    auto pathRegistry = std::make_shared<PathRegistryService>();
    ServiceLocator::Get().Register<PathRegistryService>(pathRegistry);

    auto kenneyRepo = std::make_shared<KenneyPathRepository>();
    pathRegistry->RegisterRepository<KenneyPathRepository>(kenneyRepo);
    
    // Also register Font Path Repo? Or is it automatic?
    // FontService uses FontPathRepository. Let's register it if not already?
    // In previous FontService code it looked for FontPathRepository.
    // It seems missing here! Let's add it.
    auto fontRepo = std::make_shared<FontPathRepository>();
    pathRegistry->RegisterRepository<FontPathRepository>(fontRepo);

    auto resources = std::make_shared<ResourceService>();
    ServiceLocator::Get().Register<IResourceService>(resources);

    auto shaderSystem = std::make_shared<OpenGLShaderService>();
    ServiceLocator::Get().Register<IShaderService>(shaderSystem);

    auto fontService = std::make_shared<FontService>();
    ServiceLocator::Get().Register<IFontService>(fontService);

    m_isRunning = true;
    m_lastTime = SDL_GetTicks();
}

void Engine::AttachLayer(std::shared_ptr<GameLayer> layer)
{
    m_gameLayer = layer;
    if (m_gameLayer)
        m_gameLayer->OnAttach();
}

void Engine::ToggleFullscreen()
{
    m_fullscreen = !m_fullscreen;
    SDL_SetWindowFullscreen(m_window, m_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void Engine::Run()
{
    auto inputService = std::dynamic_pointer_cast<InputService>(ServiceLocator::Get().GetService<IInputService>());
    
    while (m_isRunning)
    {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - m_lastTime) / 1000.0f;
        m_lastTime = currentTime;

        if (inputService) inputService->Update();

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) m_isRunning = false;
            
            // Window Events
            if (e.type == SDL_WINDOWEVENT)
            {
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int w = e.window.data1;
                    int h = e.window.data2;
                    glViewport(0, 0, w, h);
                    auto renderer = ServiceLocator::Get().GetService<RenderService>();
                    if (renderer) renderer->SetViewport(w, h);
                }
            }
            
            // Toggle Fullscreen (Global, outside input service)
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F11)
            {
                ToggleFullscreen();
            }

            // Global Input Event
            if (inputService) inputService->OnEvent(e);
            
            // Layer Event
            if (m_gameLayer) m_gameLayer->OnEvent(e);
        }

        // if (inputService && inputService->IsKeyDown(SDL_SCANCODE_ESCAPE)) m_isRunning = false;

        if (m_gameLayer)
        {
            m_gameLayer->OnUpdate(deltaTime);
            m_gameLayer->OnRender();
        }
    }
}

void Engine::Clean()
{
    if (m_gameLayer) m_gameLayer->OnDetach();
    ServiceLocator::Get().Clean();
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}
