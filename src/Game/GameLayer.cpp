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
#include "Engine/Services/UIService.h"
#include "Engine/Services/UIService.h"
#include "Game/GameConfig.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

GameLayer::GameLayer() : m_debugMode(0) {}

GameLayer::~GameLayer() {}

void GameLayer::OnAttach()
{
    // Initialize Game Specific Services
    auto worldService = std::make_shared<WorldService>();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    auto atlasService = std::make_shared<TextureAtlasService>();
    // Getting Repo from Locator (should be reg in Engine)
    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    auto kenneyRepo = pathRegistry->GetRepository<KenneyPathRepository>(); // Assuming we can get it or just new it if needed. 
    // Actually Game.cpp registered it. Engine should do it.
    atlasService->LoadAtlas(kenneyRepo);
    ServiceLocator::Get().Register<TextureAtlasService>(atlasService);

    auto blockRegistry = std::make_shared<BlockRegistryService>();
    ServiceLocator::Get().Register<IBlockRegistryService>(blockRegistry);

    auto chunkManager = std::make_shared<ChunkManager>();
    ServiceLocator::Get().Register<ChunkManager>(chunkManager);

    auto uiService = std::make_shared<UIService>();
    uiService->LoadLayouts("assets/ui/ui_layouts.json");
    ServiceLocator::Get().Register<IUIService>(uiService);

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

    // Initial State
    m_currentState = GameState::MainMenu;
    if (uiService) uiService->SetScene("main_menu");
}

void GameLayer::OnDetach()
{
    // Cleanup services if needed
}

void GameLayer::OnUpdate(float deltaTime)
{
    // Use IInputService interface directly
    auto inputService = ServiceLocator::Get().GetService<IInputService>();

    auto uiService = std::dynamic_pointer_cast<UIService>(ServiceLocator::Get().GetService<IUIService>());

    if (inputService)
    {
        // Toggle Styles (Global)
        if (inputService->IsKeyDown(SDL_SCANCODE_1)) m_currentStyle = RenderStyle::MINECRAFT;
        if (inputService->IsKeyDown(SDL_SCANCODE_2)) m_currentStyle = RenderStyle::BORDERLANDS;
        if (inputService->IsKeyDown(SDL_SCANCODE_3)) m_currentStyle = RenderStyle::MOCO;
        
        // Debug Modes
        if (inputService->IsKeyDown(SDL_SCANCODE_4)) m_debugMode = 0; // Normal
        if (inputService->IsKeyDown(SDL_SCANCODE_5)) m_debugMode = 1; // Texture Only
        if (inputService->IsKeyDown(SDL_SCANCODE_6)) m_debugMode = 2; // Color Only
        if (inputService->IsKeyDown(SDL_SCANCODE_7)) m_debugMode = 3; // Normals
        if (inputService->IsKeyDown(SDL_SCANCODE_8)) m_debugMode = 4; // Atlas View
        
        // State Transitions (Debounced ideally, but valid for prototype)
        static bool s_escPressed = false;
        bool escDown = inputService->IsKeyDown(SDL_SCANCODE_ESCAPE);
        if (escDown && !s_escPressed)
        {
            if (m_currentState == GameState::MainMenu) {
                m_currentState = GameState::Gameplay;
                if (uiService) uiService->SetScene("gameplay");
            } else if (m_currentState == GameState::Gameplay) {
                m_currentState = GameState::MainMenu;
                if (uiService) uiService->SetScene("main_menu");
            } else if (m_currentState == GameState::DebugOverlay) {
                m_currentState = GameState::Gameplay; // Back to game
                if (uiService) uiService->SetScene("gameplay");
            }
        }
        s_escPressed = escDown;

        static bool s_f3Pressed = false;
        bool f3Down = inputService->IsKeyDown(SDL_SCANCODE_F3);
        if (f3Down && !s_f3Pressed)
        {
             if (m_currentState == GameState::Gameplay) {
                m_currentState = GameState::DebugOverlay;
                if (uiService) uiService->SetScene("debug_overlay");
            } else if (m_currentState == GameState::DebugOverlay) {
                m_currentState = GameState::Gameplay;
                if (uiService) uiService->SetScene("gameplay");
            }
        }
        s_f3Pressed = f3Down;
        
        // Game Logic Update only if Gameplay or Debug (maybe?)
        if (m_currentState == GameState::Gameplay || m_currentState == GameState::DebugOverlay)
        {
            m_player->Update(deltaTime, inputService.get());
            m_camera->Update(m_player->GetPosition(), deltaTime, inputService.get());
        }
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
    auto uiService = ServiceLocator::Get().GetService<IUIService>();

    if (!renderer || !shaders) return;

    // Use Post Process for everything now (or at least for this pipeline refactor)
    // The user requested a Layered Rendering Pipeline, implying this structure is now standard.
    bool usePost = postProcess != nullptr; 

    // Determine Render Path based on State
    bool renderWorld = (m_currentState != GameState::MainMenu);
    bool renderUI = true;

    if (m_debugMode == 4) {
         renderer->SetClearColor(0.2f, 0.2f, 0.2f, 1.0f);
         renderer->Clear();
         RenderAtlasDebug(renderer.get());
         renderer->SwapBuffers();
         return;
    }

    // --- 1. World Pass (3D) ---
    if (renderWorld)
    {
        if (usePost) postProcess->SetTargetLayer("World");
        else glBindFramebuffer(GL_FRAMEBUFFER, 0);

        renderer->SetClearColor(0.39f, 0.58f, 0.93f, 1.0f);
        renderer->Clear();
    
    // Perspective Projection for World
    glm::mat4 projection = m_camera->GetProjectionMatrix();
    glm::mat4 view = m_camera->GetViewMatrix();

    // Pass projection to Begin (even if Chunks use Manual Uniforms, this sets context for any Sprite/Debug 3D elements)
    renderer->Begin(projection);

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

        shaders->SetMat4(m_basicShaderID, "projection", projection);
        shaders->SetMat4(m_basicShaderID, "view", view);
        shaders->SetInt(m_basicShaderID, "u_debugMode", m_debugMode);

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
    } // End if (renderWorld)

    // --- 2. UI Pass (2D) ---
    if (usePost)
    {
        postProcess->SetTargetLayer("UI");
        renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
        renderer->Clear();

        renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Transparent
        renderer->Clear();

        // Orthographic Projection for UI
        int uiW = 1280, uiH = 720;
        if (uiService) uiService->GetScreenSize(uiW, uiH);
        glm::mat4 ortho = glm::ortho(0.0f, (float)uiW, (float)uiH, 0.0f, -1.0f, 1.0f);
        renderer->Begin(ortho);

        // Setup Shader for UI (reuse basic shader, disable effects)
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f)); // Identity View
        shaders->SetBool(m_basicShaderID, "u_celEnabled", false);
        shaders->SetBool(m_basicShaderID, "u_outlinesEnabled", false);

        if (uiService) uiService->Render(renderer.get()); 
        // Example: Debug/Crosshair could go here using renderer->DrawSprite(...)
        
        renderer->End();
    }

    // --- 3. Composite Pass ---
    if (usePost && renderWorld)
    {
        postProcess->Unbind(); 
        renderer->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        renderer->Clear();
        
        glDisable(GL_DEPTH_TEST);
        
        // 3a. Draw World (with Post Processing)
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
                default: 
                    // For Minecraft/Simple, just render texture without edges (set high thresholds)
                    normalThresh = 10.0f; 
                    depthThresh = 10.0f;
                    break;
            }

            postProcess->RenderLayerWithEdges("World", shaders, m_postEdgeShaderID, normalThresh, depthThresh, outlineColor);
        }
        
        // 3b. Draw UI (Overlay)
        // Note: We render UI in a separate pass above (to texture 'UI').
        // If MainMenu, we might just want to blit the UI texture to screen (no world).
    }

    // --- Final Screen Output ---
    // If not rendering world (MainMenu), we just need to put UI texture on screen?
    // Or if we did the UI pass, we can just blit it.
    // Actually, let's keep it simple:
    
    // If MainMenu:
    if (m_currentState == GameState::MainMenu && usePost)
    {
         postProcess->Unbind();
         renderer->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
         renderer->Clear();
         
         // Just draw UI Texture
         
         // Fix: Disable Depth Test for 2D Blit to ensure it draws over clear color
         glDisable(GL_DEPTH_TEST);
         glDisable(GL_CULL_FACE); // Ensure quad is not culled
         
         static int blitLog = 0;
         if (blitLog++ % 600 == 0) std::cout << "[GameLayer] Blitting UI to Screen (MainMenu)" << std::endl;

         shaders->UseShader(m_basicShaderID);
         shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f));
         shaders->SetMat4(m_basicShaderID, "projection", glm::mat4(1.0f));
         shaders->SetBool(m_basicShaderID, "u_celEnabled", false);
         
         renderer->Begin(glm::mat4(1.0f)); // Identity Projection
         
         Sprite uiFSQ;
         uiFSQ.Size = glm::vec2(2,2); 
         uiFSQ.TextureID = postProcess->GetTexture("UI");
         uiFSQ.Color = glm::vec4(1.0f);
         renderer->DrawSprite(uiFSQ);
         renderer->End();
    }
    else if (usePost && renderWorld)
    {
        // We already did Unbind() and World Composite in block above.
        // Now add UI on top.
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Use Batch Renderer to draw the UI Texture as a full-screen quad
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f));
        shaders->SetMat4(m_basicShaderID, "projection", glm::mat4(1.0f)); // NDC
        shaders->SetBool(m_basicShaderID, "u_celEnabled", false);

        renderer->Begin(glm::mat4(1.0f));
        
        Sprite uiFSQ;
        uiFSQ.Position = glm::vec2(0,0);
        uiFSQ.Size = glm::vec2(2,2); // Covers -1 to 1
        uiFSQ.TextureID = postProcess->GetTexture("UI");
        uiFSQ.Color = glm::vec4(1.0f);
        
        renderer->DrawSprite(uiFSQ);
        renderer->End();
        
        glEnable(GL_DEPTH_TEST); 
    }
    else if (!usePost)
    {
        // Fallback if no PostProcess (Direct Render)
        // We probably don't support this path well with the new UI design but just in case.
        int uiW = 1280, uiH = 720;
        if (uiService) uiService->GetScreenSize(uiW, uiH);
        
        renderer->Begin(glm::ortho(0.0f, (float)uiW, (float)uiH, 0.0f, -1.0f, 1.0f));
        if (uiService) uiService->Render(renderer.get());
        renderer->End();
    } 
    
    renderer->SwapBuffers();
}

void GameLayer::RenderAtlasDebug(RenderService* renderer) {
   // Draw Atlas centered
   renderer->Begin(glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f, -1.0f, 1.0f));
   
   Sprite atlasSprite;
   atlasSprite.Position = glm::vec2(640, 360);
   atlasSprite.Size = glm::vec2(500, 500 * 2); // Aspect 1:2 approx
   atlasSprite.TextureID = m_textureID; // The Atlas
   atlasSprite.Color = glm::vec4(1.0f);
   
   renderer->DrawSprite(atlasSprite);
   renderer->End();
}
