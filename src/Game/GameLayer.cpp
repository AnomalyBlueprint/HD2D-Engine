#include "Game/GameLayer.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/IInputService.h"
#include "Engine/Services/IWorldService.h"
#include "Engine/Services/ChunkManager.h"
#include "Engine/Services/BlockRegistryService.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/PostProcessService.h"
#include "Engine/Services/KenneyPathRepository.h"
#include "Engine/Services/WorldService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/InputService.h"
#include "Game/GameConfig.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

GameLayer::GameLayer() {}

GameLayer::~GameLayer() {}

void GameLayer::OnAttach()
{
    // Initialize Game Specific Services
    auto worldService = std::make_shared<WorldService>();
    worldService->Init();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    auto atlasService = std::make_shared<TextureAtlasService>();
    // Getting Repo from Locator (should be reg in Engine)
    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    auto kenneyRepo = pathRegistry->GetRepository<KenneyPathRepository>(); // Assuming we can get it or just new it if needed. 
    // Actually Game.cpp registered it. Engine should do it.
    atlasService->LoadAtlas(kenneyRepo);
    ServiceLocator::Get().Register<TextureAtlasService>(atlasService);

    auto blockRegistry = std::make_shared<BlockRegistryService>();
    blockRegistry->Init();
    ServiceLocator::Get().Register<IBlockRegistryService>(blockRegistry);

    auto chunkManager = std::make_shared<ChunkManager>();
    chunkManager->Init();
    ServiceLocator::Get().Register<ChunkManager>(chunkManager);

    // Get Window size for PostProcess (Optional: Engine could pass it)
    int w = 1280, h = 720; // TODO: Get from Engine/Window
    SDL_Window* window = SDL_GL_GetCurrentWindow(); // Hacky but works if context is current
    if (window) SDL_GetWindowSize(window, &w, &h);
    
    auto postProcess = std::make_shared<PostProcessService>(w, h);
    ServiceLocator::Get().Register<PostProcessService>(postProcess);

    // Player & Camera
    float aspect = (float)w / (float)h;
    m_camera = std::make_shared<Camera>(aspect);
    m_camera->target = glm::vec3(0.0f, 0.0f, 0.0f); 
    m_camera->distance = 1500.0f;
    m_camera->orbitAngle = 0.0f;

    m_player = std::make_unique<Player>();
    m_player->Init();

    // Shaders
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    m_basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    m_postEdgeShaderID = shaders->LoadShader("assets/shaders/post_edge.vert", "assets/shaders/post_edge.frag");

    m_textureID = atlasService->GetTextureID();
}

void GameLayer::OnDetach()
{
    // Cleanup services if needed
}

void GameLayer::OnUpdate(float deltaTime)
{
    auto inputService = std::dynamic_pointer_cast<InputService>(ServiceLocator::Get().GetService<IInputService>()); // Cast for Update? Interface calls?
    // Game.cpp casted to InputService for IsKeyDown which should be in Interface... 
    // Actually IInputService has IsKeyDown. 
    // Game.cpp casted it... let's check. 
    // I will use IInputService.

    if (inputService)
    {
        if (inputService->IsKeyDown(SDL_SCANCODE_1)) m_currentStyle = RenderStyle::MINECRAFT;
        if (inputService->IsKeyDown(SDL_SCANCODE_2)) m_currentStyle = RenderStyle::BORDERLANDS;
        if (inputService->IsKeyDown(SDL_SCANCODE_3)) m_currentStyle = RenderStyle::MOCO;
        
        m_player->Update(deltaTime, inputService.get());
        m_camera->Update(m_player->GetPosition(), deltaTime, inputService.get());
    }

    // Update Title (Need Window access? Engine handles title? Or GameLayer sets it via SDL)
    // For now, let's skip title update or do it here.
    static Uint32 lastTitleUpdate = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime > lastTitleUpdate + 100)
    {
        lastTitleUpdate = currentTime;
        glm::vec3 pPos = m_player->GetPosition();
        int cx = (int)floor(pPos.x / GameConfig::CHUNK_PIXEL_SIZE);
        int cz = (int)floor(pPos.z / GameConfig::CHUNK_PIXEL_SIZE);
        float fps = (deltaTime > 0) ? 1.0f / deltaTime : 0.0f;
        std::string styleName = "BORDERLANDS";
        if (m_currentStyle == RenderStyle::MINECRAFT) styleName = "MINECRAFT";
        if (m_currentStyle == RenderStyle::MOCO) styleName = "MOCO";
        
        std::string title = "HD2D Engine | Style: " + styleName + " | Chunk: [" + std::to_string(cx) + ", " + std::to_string(cz) + "] | FPS: " + std::to_string((int)fps);
        SDL_Window* curWindow = SDL_GL_GetCurrentWindow();
        if (curWindow) SDL_SetWindowTitle(curWindow, title.c_str());
    }
}

void GameLayer::OnEvent(SDL_Event& e)
{
    // Input service mostly handles this via Engine, but if we need specific events
}

void GameLayer::OnRender()
{
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();
    auto postProcess = ServiceLocator::Get().GetService<PostProcessService>();

    if (!renderer || !shaders) return;

    bool usePost = (m_currentStyle != RenderStyle::MINECRAFT);

    // 1. Off-screen Rendering Pass
    if (usePost && postProcess) 
    {
        postProcess->Bind();
    }
    else 
    {
            // Render directly to screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }
    
    renderer->Clear();
    renderer->Begin();

    if (m_basicShaderID > 0)
    {
        shaders->UseShader(m_basicShaderID);

        // --- Cel Shading Uniforms ---
        float time = SDL_GetTicks() / 1000.0f;
        float angle = time * (glm::two_pi<float>() / GameConfig::DAY_CYCLE_SPEED);
        float lightX = sin(angle);
        float lightZ = cos(angle);
        glm::vec3 lightDir = glm::normalize(glm::vec3(lightX, 1.0f, lightZ));

        shaders->SetVec3(m_basicShaderID, "u_lightDir", lightDir);
        
        // --- Style Specific Uniforms ---
        int lightBands = 256;
        float rimStrength = 0.0f;

        switch (m_currentStyle) {
            case RenderStyle::MINECRAFT:
                lightBands = 256; 
                rimStrength = 0.0f;
                break;
            case RenderStyle::BORDERLANDS:
                lightBands = 2; 
                rimStrength = 0.1f;
                break;
            case RenderStyle::MOCO:
                lightBands = 4; 
                rimStrength = 0.6f;
                break;
        }

        shaders->SetInt(m_basicShaderID, "u_lightBands", lightBands);
        shaders->SetFloat(m_basicShaderID, "u_rimStrength", rimStrength);
        shaders->SetVec3(m_basicShaderID, "u_viewPos", m_camera->GetPosition());

        shaders->SetFloat(m_basicShaderID, "u_outlineThickness", GameConfig::CelShading::OUTLINE_WIDTH);
        shaders->SetFloat(m_basicShaderID, "u_ambientStrength", GameConfig::CelShading::AMBIENT_STRENGTH);
        shaders->SetBool(m_basicShaderID, "u_celEnabled", GameConfig::CelShading::ENABLED);
        shaders->SetBool(m_basicShaderID, "u_outlinesEnabled", GameConfig::CelShading::OUTLINES_ENABLED);

        glm::mat4 projection = m_camera->GetProjectionMatrix();
        glm::mat4 view = m_camera->GetViewMatrix(); 
        
        shaders->SetMat4(m_basicShaderID, "projection", projection);
        shaders->SetMat4(m_basicShaderID, "view", view);

        if (chunkManager)
        {
            chunkManager->Update(m_player->GetPosition());
            renderer->UseTexture(m_textureID);
            chunkManager->Render(renderer.get(), shaders.get());
        }
        
        if (m_player)
        {
            renderer->UseTexture(0); 
            m_player->Render(renderer.get(), shaders.get(), m_basicShaderID);
        }
    }
    
    renderer->End();

    // 2. Post-Processing Pass
    if (usePost && postProcess)
    {
        postProcess->Unbind(); // Switch back to default framebuffer
        renderer->Clear(); // Clear the screen
        
        glDisable(GL_DEPTH_TEST);
        
        if (m_postEdgeShaderID > 0)
        {
            float normalThresh = 0.4f;
            float depthThresh = 0.02f;
            glm::vec4 outlineColor(0,0,0,1);

            switch (m_currentStyle) {
                case RenderStyle::BORDERLANDS:
                    normalThresh = 0.4f;
                    depthThresh = 0.02f;
                    outlineColor = glm::vec4(0,0,0,1);
                    break;
                case RenderStyle::MOCO:
                    normalThresh = 0.6f;
                    depthThresh = 0.02f; 
                    outlineColor = glm::vec4(0.2f, 0.1f, 0.1f, 1.0f);
                    break;
                default: break;
            }

            postProcess->RenderRect(shaders, m_postEdgeShaderID, normalThresh, depthThresh, outlineColor);
        }
        
        glEnable(GL_DEPTH_TEST); 
    }
    
    renderer->SwapBuffers();
}
