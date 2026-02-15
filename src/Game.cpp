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
// NEW INCLUDES
#include "Services/PathRegistryService.h"
#include "Services/KenneyPathRepository.h"
#include "Services/TextureAtlasService.h" // Added
#include "Services/WorldService.h" // Added
#include "Services/ChunkManager.h" // Added
#include "World/Chunk.h" // Added
#include "Data/KenneyIDs.h"

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/Sprite.h"

void Game::Init()
{
    // 1. Logger (First!)
    auto loggerServ = std::make_shared<LoggerService>();
    ServiceLocator::Get().Register<ILoggerService>(loggerServ);

    // Helper lambda
    auto logger = []()
    { return ServiceLocator::Get().GetService<ILoggerService>(); };

    logger()->Log("Engine Starting...");

    // 2. SDL Init
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        logger()->LogError("SDL Init Failed!");
        return;
    }

    // 3. OpenGL Settings & Window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("HD-2D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // --- CAMERA SETUP ---
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    camera = std::make_shared<Camera>(aspect);
    
    // FIX 1: Center position
    // Chunk is 16 blocks * 32 pixels = 512 pixels wide/deep.
    // Center is 256.
    camera->position = glm::vec2(256.0f, 256.0f); 

    // FIX 2: Massive Zoom Out
    // We want to see about 20 blocks vertical (20 * 32 = 640 pixels).
    // If your Camera math is "height = 2.0f * zoom", then zoom needs to be ~320.
    camera->zoom = 0.20f;
    // --------------------

    // 4. Render Service (CRITICAL: This initializes GLEW!)
    auto renderer = std::make_shared<OpenGLRenderService>(window);
    ServiceLocator::Get().Register<RenderService>(renderer);

    // 5. Input Service
    auto input = std::make_shared<InputService>();
    ServiceLocator::Get().Register<IInputService>(input);

    // 6. Resource Service
    auto resources = std::make_shared<ResourceService>();
    ServiceLocator::Get().Register<IResourceService>(resources);

    // 7. Path Registry & Kenney Repo
    auto pathRegistry = std::make_shared<PathRegistryService>();
    ServiceLocator::Get().Register<PathRegistryService>(pathRegistry);

    auto kenneyRepo = std::make_shared<KenneyPathRepository>();
    pathRegistry->RegisterRepository<KenneyPathRepository>(kenneyRepo);

    // 8. Shader Service
    auto shaderSystem = std::make_shared<OpenGLShaderService>();
    ServiceLocator::Get().Register<IShaderService>(shaderSystem);
    
    // 9. World Service (NEW)
    auto worldService = std::make_shared<WorldService>();
    worldService->Init();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    // 10. Texture Atlas Service (NEW)
    auto atlasService = std::make_shared<TextureAtlasService>();
    atlasService->LoadAtlas(kenneyRepo);
    ServiceLocator::Get().Register<TextureAtlasService>(atlasService);

    // 11. Chunk Manager (NEW)
    auto chunkManager = std::make_shared<ChunkManager>();
    chunkManager->Init();
    ServiceLocator::Get().Register<ChunkManager>(chunkManager);

    // Test Shaders
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // Load Texture via KenneyIDs
    // We now use Atlas (atlasService->GetTextureID())
    textureID = atlasService->GetTextureID();

    // --- VOXEL CHUNK GENERATION (Removed - Handled by ChunkManager) ---
    // Chunk* chunk = new Chunk(); ...

    // debugMeshID = 0; // Disable debug cube

    lastTime = SDL_GetTicks();
    isRunning = true;
    logger()->Log("Game Initialized.");
}

void Game::Run()
{
    // Need to cast IInputService back to InputService to call OnEvent
    // Or we should expose OnEvent in IInputService? 
    // Ideally InputService handles SDL_PumpEvents internally in Update,
    // but SDL_PollEvent is usually done in the main loop to handle OS window events (Quit, Resize).
    // So we pass events to InputService.
    auto inputService = std::dynamic_pointer_cast<InputService>(ServiceLocator::Get().GetService<IInputService>());

    while (isRunning)
    {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // Convert ms to seconds
        lastTime = currentTime;
        
        // 1. Input Update
        if (inputService) inputService->Update();

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                isRunning = false;
            
            // Pass event to input service (for scrolling)
            if (inputService) inputService->OnEvent(e);
        }

        if (inputService && inputService->IsKeyDown(SDL_SCANCODE_ESCAPE))
            isRunning = false;

        // 2. Zoom Logic
        if (inputService)
        {
            float previousZoom = camera->zoom;
            // User requested gentle speed: zoom * 2.0f * deltaTime
            float zoomChange = camera->zoom * 2.0f * deltaTime;

            // Scroll Zoom (Discrete steps but scaled)
            int scroll = inputService->GetMouseScroll();
            // Scroll also uses proportional change? 
            // "Speed: camera->zoom * 2.0f * deltaTime" applies to continuous keys?
            // For scroll, let's use a small percentage per click, e.g. 10%.
            if (scroll != 0) camera->zoom -= scroll * (camera->zoom * 0.1f); 

            // Key Zoom (Q/E)
            if (inputService->IsKeyDown(SDL_SCANCODE_Q)) camera->zoom -= zoomChange; // Zoom In
            if (inputService->IsKeyDown(SDL_SCANCODE_E)) camera->zoom += zoomChange; // Zoom Out

            // Clamp (0.15 to 0.25)
            if (camera->zoom < 0.15f) camera->zoom = 0.15f;
            if (camera->zoom > 1.0f) camera->zoom = 1.0f;

            // Debug Output
            if (abs(camera->zoom - previousZoom) > 0.0001f)
            {
                std::cout << "[DEBUG] Current Zoom Level: " << camera->zoom << std::endl;
            }
        }

        // 3. Render Services
        auto renderer = ServiceLocator::Get().GetService<RenderService>();
        auto shaders = ServiceLocator::Get().GetService<IShaderService>();

        // 3. Simple Camera Movement (Arrow Keys / WASD)
        
        // Speed needs to be higher now that we are in Pixel Coordinates
        float speed = 300.0f * deltaTime; // Move 300 pixels per second
        
        if (inputService->IsKeyDown(SDL_SCANCODE_W)) camera->position.y += speed;
        if (inputService->IsKeyDown(SDL_SCANCODE_S)) camera->position.y -= speed;
        if (inputService->IsKeyDown(SDL_SCANCODE_A)) camera->position.x -= speed;
        if (inputService->IsKeyDown(SDL_SCANCODE_D)) camera->position.x += speed;

        // Limit Camera (Keep Chunk in View)
        // Chunk is 0..512 in X and Y.
        // Let's allow seeing a bit outside (-500 to 1000)
        if (camera->position.x < -500.0f) camera->position.x = -500.0f;
        if (camera->position.x > 1000.0f) camera->position.x = 1000.0f;
        if (camera->position.y < -500.0f) camera->position.y = -500.0f;
        if (camera->position.y > 1000.0f) camera->position.y = 1000.0f;

        // 4. Render
        if (renderer)
        {
            renderer->Clear();
            renderer->Begin();

            if (basicShaderID > 0)
            {
                shaders->UseShader(basicShaderID);

                // Update Camera Matrices
                glm::mat4 projection = camera->GetProjectionMatrix();
                shaders->SetMat4(basicShaderID, "projection", projection);
                
                // ISOMETRIC VIEW SETUP
                // 1. Get Base View (Translation)
                glm::mat4 view = camera->GetViewMatrix();
                
                // 2. Apply Isometric Rotation
                // 2. Apply Isometric Rotation
                view = glm::rotate(view, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
                view = glm::rotate(view, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                int viewLoc = glGetUniformLocation(basicShaderID, "view");
                int projLoc = glGetUniformLocation(basicShaderID, "projection");
                int modelLoc = glGetUniformLocation(basicShaderID, "model"); 

                if (viewLoc >= 0) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
                if (projLoc >= 0) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
                
                // Set Identity Model for default
                glm::mat4 identityModel(1.0f);
                if (modelLoc >= 0) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &identityModel[0][0]);

                // --- CHUNK MANAGER ---
                auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();
                if (chunkManager)
                {
                    chunkManager->Update(camera->position);
                    
                    // Bind Atlas
                    renderer->UseTexture(textureID);
                    
                    chunkManager->Render(renderer.get(), nullptr); 
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