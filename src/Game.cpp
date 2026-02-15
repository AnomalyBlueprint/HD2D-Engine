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

    // Test Shaders
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // Load Texture via KenneyIDs
    std::string texturePath = kenneyRepo->GetPath((int)KenneyIDs::Floor_Ground_Dirt);
    if (!texturePath.empty())
    {
        textureID = resources->GetTexture(texturePath);
    }

    // --- VOXEL CHUNK GENERATION ---
    Chunk* chunk = new Chunk();
    worldService->GenerateChunk(chunk, 0, 0);
    
    std::vector<Vertex> chunkVertices;
    std::vector<unsigned int> chunkIndices;
    
    chunk->RebuildMesh(chunkVertices, chunkIndices);
    
    // We need CreateMesh to accept vector<Vertex>.
    // But CreateMesh interface accepts vector<float>.
    // Wait, I updated CreateMesh to EXPECT floats but use sizeof(Vertex).
    // So I need to cast/copy Vertex vector to float vector?
    // OR Update RenderService to accept vector<Vertex>?
    // The user instruction said: "Refactor OpenGLRenderService and Game.cpp to implement these fixes."
    // But CreateMesh signature in RenderService.h was:
    // unsigned int CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
    // I should cast it.
    
    // Reinterpret cast is dangerous if padding exists, but our struct is packed (floats).
    // 3+4+2+1 = 10 floats. 10 * 4 = 40 bytes.
    // Alignment is likely 4 bytes.
    
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
    
    // Wait, can I overload CreateMesh?
    // The prompt for Task 2 said "Upload to GPU: chunkMeshID = renderer->CreateMesh(vertices, indices);"
    // implying passing `vector<Vertex>`.
    // I should probably overload CreateMesh in RenderService to accept `vector<Vertex>`.
    // But I can't change RenderService interface easily without re-compiling everything.
    // For now, I will do the conversion here to keep it simple and compile-safe.
    
    chunkIndexCount = chunkIndices.size();
    chunkMeshID = renderer->CreateMesh(floatVertices, chunkIndices);
    
    logger()->Log("Chunk Generated. Indices: " + std::to_string(chunkIndexCount));
    
    // --- DEBUG CUBE (Task 3) ---
    // Position: (8, 8, 0)
    // Size: 1.0? 32.0? "Simple Test Cube". Let's assume 1 unit size (or 32 if scaled?).
    // "Position: (8, 8, 0)". If I use 32.0 scale, 8 is tiny. 
    // But if Voxel World is 0..16, and I want to see this cube, maybe it IS 1 unit.
    // Or maybe 8,8,0 means 8 blocks, 8 blocks?
    // Let's use 10.0f size for visibility.
    
    // Vertex Format: Pos(3), Color(4), UV(2), TexID(1)
    // Red: 1,0,0,1. TexID: 0 (White Texture).
    
    float size = 10.0f;
    float x = 8.0f; float y = 8.0f; float z = 0.0f;
    
    std::vector<float> debugCubeVerts = {
        // Front Face
        x, y, z+size,         1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  0.0f,
        x+size, y, z+size,    1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 0.0f,  0.0f,
        x+size, y+size, z+size, 1.0f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f,  0.0f,
        x, y+size, z+size,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  0.0f,
        // (Just one face for now is enough to see it, but let's do a cube?)
        // Let's just do a Quad for simplicity given the "Test Cube" request often implies just seeing SOMETHING.
        // Okay, let's do a full cube (6 faces * 4 verts = 24 verts).
        // Actually, CreateMesh expects indices.
    };
    
    // Let's just create a single Quad at 8,8,0 facing camera to be sure.
    // Wait, Isometric view. A cube is better.
    // I'll make a helper function? No, inline for speed.
    // ...
    // Let's just utilize the CHUNK logic to spawn a single block chunk? 
    // No, user wants MANUAL creation.
    
    // OK, MANUAL CUBE VERTICES (Front Face only for brevity, user just wants to see it)
    // Wait, "Test Cube". I should verify 3D.
    // I will add TOP and SIDE faces too.
    
    // Front (Z+)
    // Top (Y+)
    // Right (X+)
    
    auto PushQuad = [&](float x, float y, float z, int axis, float r, float g, float b) {
       // axis 0=Z (Front), 1=Y (Top), 2=X (Right)
       // ...
       // Simplified: Just 3 faces.
       // Front
       debugCubeVerts.insert(debugCubeVerts.end(), {
           x, y, z+size, r,g,b,1, 0,0,0,
           x+size,y,z+size, r,g,b,1, 1,0,0,
           x+size,y+size,z+size, r,g,b,1, 1,1,0,
           x,y+size,z+size, r,g,b,1, 0,1,0
       });
    };
    
    PushQuad(x,y,z, 0, 1.0f, 0.0f, 0.0f); // Front Red
    
    std::vector<unsigned int> debugIndices = {0, 1, 2, 2, 3, 0};
    debugIndexCount = debugIndices.size();
    debugMeshID = renderer->CreateMesh(debugCubeVerts, debugIndices);
    
    logger()->Log("Debug Cube Created.");

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
                    renderer->UseTexture(0); // Use White Texture (Colors come from Vertices)
                    renderer->DrawMesh(chunkMeshID, chunkIndexCount);
                }

                // Draw Debug Cube (Task 3)
                if (debugMeshID > 0)
                {
                    renderer->UseTexture(0); // Use White Texture (Default)
                    renderer->DrawMesh(debugMeshID, debugIndexCount);
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