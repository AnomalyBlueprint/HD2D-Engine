#include "Game.h"
#include "Services/ServiceLocator.h"
#include "Services/RenderService.h"
#include "Services/OpenGLRenderService.h"
#include "Services/ILoggerService.h"
#include "Services/LogTypes.h"
#include "Services/LoggerService.h"
#include "Services/IShaderService.h"
#include "Services/OpenGLShaderService.h"
#include "Services/IInputService.h" 
#include "Services/InputService.h"   
#include "Services/IResourceService.h" 
#include "Services/ResourceService.h" 
#include "Services/PathRegistryService.h"
#include "Services/KenneyPathRepository.h"
#include "Services/TextureAtlasService.h" 
#include "Services/WorldService.h" 
#include "Services/ChunkManager.h" 
#include "World/Chunk.h" 
#include "Data/KenneyIDs.h"
#include "Core/GameConfig.h"
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/Sprite.h"
#include "Core/Player.h" 

void Game::Init()
{
    auto loggerServ = std::make_shared<LoggerService>();
    ServiceLocator::Get().Register<ILoggerService>(loggerServ);

    auto logger = [](){ return ServiceLocator::Get().GetService<ILoggerService>(); };
    logger()->Log("Engine Starting...");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        logger()->LogError("SDL Init Failed!");
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("HD-2D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    camera = std::make_shared<Camera>(aspect);
    camera->target = glm::vec3(0.0f, 0.0f, 0.0f); 
    camera->distance = 800.0f;
    camera->orbitAngle = 0.0f;

    auto renderer = std::make_shared<OpenGLRenderService>(window);
    ServiceLocator::Get().Register<RenderService>(renderer);

    auto input = std::make_shared<InputService>();
    ServiceLocator::Get().Register<IInputService>(input);

    auto resources = std::make_shared<ResourceService>();
    ServiceLocator::Get().Register<IResourceService>(resources);

    auto pathRegistry = std::make_shared<PathRegistryService>();
    ServiceLocator::Get().Register<PathRegistryService>(pathRegistry);

    auto kenneyRepo = std::make_shared<KenneyPathRepository>();
    pathRegistry->RegisterRepository<KenneyPathRepository>(kenneyRepo);

    auto shaderSystem = std::make_shared<OpenGLShaderService>();
    ServiceLocator::Get().Register<IShaderService>(shaderSystem);
    
    auto worldService = std::make_shared<WorldService>();
    worldService->Init();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    auto atlasService = std::make_shared<TextureAtlasService>();
    atlasService->LoadAtlas(kenneyRepo);
    ServiceLocator::Get().Register<TextureAtlasService>(atlasService);

    auto chunkManager = std::make_shared<ChunkManager>();
    chunkManager->Init();
    ServiceLocator::Get().Register<ChunkManager>(chunkManager);

    m_player = std::make_unique<Player>();
    m_player->Init();

    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    textureID = atlasService->GetTextureID();

    lastTime = SDL_GetTicks();
    isRunning = true;
    logger()->Log("Game Initialized.");
}

void Game::Run()
{
    auto inputService = std::dynamic_pointer_cast<InputService>(ServiceLocator::Get().GetService<IInputService>());
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();

    while (isRunning)
    {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; 
        lastTime = currentTime;
        
        if (inputService) inputService->Update();

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) isRunning = false;
            if (inputService) inputService->OnEvent(e);
        }

        if (inputService && inputService->IsKeyDown(SDL_SCANCODE_ESCAPE)) isRunning = false;

        if (inputService)
        {
            m_player->Update(deltaTime, inputService.get());
            camera->Update(m_player->GetPosition(), deltaTime, inputService.get());
        }

        static Uint32 lastTitleUpdate = 0;
        if (currentTime > lastTitleUpdate + 100)
        {
            lastTitleUpdate = currentTime;
            glm::vec3 pPos = m_player->GetPosition();
            int cx = (int)floor(pPos.x / GameConfig::CHUNK_PIXEL_SIZE);
            int cz = (int)floor(pPos.z / GameConfig::CHUNK_PIXEL_SIZE);
            float fps = (deltaTime > 0) ? 1.0f / deltaTime : 0.0f;
            std::string title = "HD2D Engine | Chunk: [" + std::to_string(cx) + ", " + std::to_string(cz) + "] | FPS: " + std::to_string((int)fps);
            SDL_SetWindowTitle(window, title.c_str());
        }

        if (renderer)
        {
            renderer->Clear();
            renderer->Begin();

            if (basicShaderID > 0)
            {
                shaders->UseShader(basicShaderID);

                // --- FIX: Send BOTH View and Projection Matrices ---
                glm::mat4 projection = camera->GetProjectionMatrix();
                glm::mat4 view = camera->GetViewMatrix(); 
                
                shaders->SetMat4(basicShaderID, "projection", projection);
                shaders->SetMat4(basicShaderID, "view", view); // <--- THIS WAS MISSING BEFORE

                if (chunkManager)
                {
                    chunkManager->Update(m_player->GetPosition());
                    renderer->UseTexture(textureID);
                    chunkManager->Render(renderer.get(), shaders.get());
                }
                
                if (m_player)
                {
                    renderer->UseTexture(0); 
                    m_player->Render(renderer.get(), shaders.get(), basicShaderID);
                }
            }
            
            renderer->End();
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