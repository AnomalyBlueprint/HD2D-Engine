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
    // Safety check
    if (h == 0)
        h = 1;
    float aspect = (float)w / (float)h;

    camera = std::make_shared<Camera>(aspect);
    camera->position = glm::vec2(8.0f, 8.0f); 
    camera->zoom = 15.0f; // Task 3: Zoom 15.0
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

    // Test Shaders
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // Load Texture via KenneyIDs
    // We now use Atlas (atlasService->GetTextureID())
    textureID = atlasService->GetTextureID();

    // --- VOXEL CHUNK GENERATION ---
    Chunk* chunk = new Chunk();
    worldService->GenerateChunk(chunk, 0, 0);
    
    std::vector<Vertex> chunkVertices;
    std::vector<unsigned int> chunkIndices;
    
    chunk->RebuildMesh(chunkVertices, chunkIndices, atlasService.get());
    
    // Safer:
    std::vector<float> floatVertices;
    floatVertices.reserve(chunkVertices.size() * 10);
    for(const auto& v : chunkVertices)
    {
        floatVertices.push_back(v.position.x); floatVertices.push_back(v.position.y); floatVertices.push_back(v.position.z);
        floatVertices.push_back(v.color.r); floatVertices.push_back(v.color.g); floatVertices.push_back(v.color.b); floatVertices.push_back(v.color.a);
        floatVertices.push_back(v.texCoord.x); floatVertices.push_back(v.texCoord.y);
        floatVertices.push_back(v.textureID);
    }
    
    chunkIndexCount = chunkIndices.size();
    chunkMeshID = renderer->CreateMesh(floatVertices, chunkIndices);
    
    logger()->Log("Chunk Generated. Indices: " + std::to_string(chunkIndexCount));
    
    // --- DEBUG CUBE (Task 3) ---
    // REMOVED (Replaced by Atlas rendering integration)
    
    /*
    float size = 10.0f;
    float x = 8.0f; float y = 8.0f; float z = 0.0f;
    ...
    */
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
            int scroll = inputService->GetMouseScroll();
            if (scroll != 0)
            {
                // Scroll UP (+1) -> Zoom IN (Value decreases)
                // Scroll DOWN (-1) -> Zoom OUT (Value increases)
                camera->zoom -= scroll * 0.1f;
                // Clamp zoom
                if (camera->zoom < 0.1f) camera->zoom = 0.1f;
                if (camera->zoom > 5.0f) camera->zoom = 5.0f;
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
                shaders->SetMat4(basicShaderID, "projection", camera->GetProjectionMatrix());
                
                // ISOMETRIC VIEW SETUP
                // 1. Get Base View (Translation)
                glm::mat4 view = camera->GetViewMatrix();
                
                // 2. Apply Isometric Rotation
                // Rotate around X axis (Pitch) approx 30-45 degrees to look down
                // Rotate around Y axis (Yaw) 45 degrees to look diagonal
                
                // Note: We need to rotate around the CENTER of the screen/camera focus?
                // Or just rotate the camera orientation.
                // Camera View Matrix = Inverse of Camera Transform.
                // If Camera is rotated X=30, Y=45... View Matrix rotates -45 Y, -30 X.
                
                view = glm::rotate(view, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
                view = glm::rotate(view, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                shaders->SetMat4(basicShaderID, "view", view);

                // Draw Floor - DISABLED for Voxel View
                // Draw Chunk
                if (chunkMeshID > 0)
                {
                    renderer->UseTexture(textureID); // Bind Atlas
                    renderer->DrawMesh(chunkMeshID, chunkIndexCount);
                }

                /* Debug Cube Removed */
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