#include "Game/GameLayer.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/ILoggerService.h"
#include "Engine/Services/IInputService.h"
#include "Engine/Services/IWorldService.h"
#include "Engine/Services/IMacroService.h"
#include "Engine/Services/MacroService.h"
#include "Engine/Services/ChunkManager.h"
#include "Engine/Services/BlockRegistryService.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/PostProcessService.h"
#include "Engine/Services/KenneyPathRepository.h"
#include "Engine/Services/WorldService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/InputService.h"
#include "Engine/Services/UIService.h"
#include "Engine/Services/DatabaseService.h"
#include "Game/GameConfig.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

GameLayer::GameLayer() : m_debugMode(0) {}

GameLayer::~GameLayer() {}

void GameLayer::OnAttach()
{
    // Initialize Game Specific Services
    // Initialize Game Specific Services
    auto worldService = std::make_shared<WorldService>();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    auto macroService = std::make_shared<MacroService>();
    ServiceLocator::Get().Register<IMacroService>(macroService);

    auto atlasService = std::make_shared<TextureAtlasService>();
    // Getting Repo from Locator (should be reg in Engine)
    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    auto kenneyRepo = pathRegistry->GetRepository<KenneyPathRepository>(); // Assuming we can get it or just new it if needed. 
    // Actually Game.cpp registered it. Engine should do it.
    atlasService->LoadAtlas(kenneyRepo);
    ServiceLocator::Get().Register<TextureAtlasService>(atlasService);

    auto blockRegistry = std::make_shared<BlockRegistryService>();
    ServiceLocator::Get().Register<IBlockRegistryService>(blockRegistry);

    auto dbService = std::make_shared<DatabaseService>();
    if (!dbService->InitStatic("assets/data/epoch.db")) ServiceLocator::Get().GetService<ILoggerService>()->Log("Warning: Static DB Not Found (Or failed to load).");
    ServiceLocator::Get().Register<DatabaseService>(dbService);

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
    SwitchScene(GameState::MainMenu);
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
        // Toggle Styles (Global) - REMOVED (Moved to UI)
        // Debug Modes - REMOVED (Moved to UI)
        
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

        // F3 Toggle Removed - Moved to UI (Generate World -> Debug Overlay)
        
        // --- Debug Scene Switching ---
        if (inputService->IsKeyDown(SDL_SCANCODE_F1)) SwitchScene(GameState::MainMenu);
        if (inputService->IsKeyDown(SDL_SCANCODE_F2)) SwitchScene(GameState::Gameplay);
        if (inputService->IsKeyDown(SDL_SCANCODE_F3)) SwitchScene(GameState::DebugOverlay);
        if (inputService->IsKeyDown(SDL_SCANCODE_F4)) SwitchScene(GameState::CharacterCreation);
        if (inputService->IsKeyDown(SDL_SCANCODE_F5)) SwitchScene(GameState::PixelForge);
        if (inputService->IsKeyDown(SDL_SCANCODE_F6)) SwitchScene(GameState::TradeInterface);
        if (inputService->IsKeyDown(SDL_SCANCODE_F7)) SwitchScene(GameState::DialogueWindow);
        if (inputService->IsKeyDown(SDL_SCANCODE_F8)) SwitchScene(GameState::DeathSummary);
        
        // Map Mode Toggles - REMOVED (Moved to UI)
        
        if (m_currentState == GameState::Gameplay)
        {
            m_player->Update(deltaTime, inputService.get());
            m_camera->Update(m_player->GetPosition(), deltaTime, inputService.get());
        }

        // GUI Interactions
        if (uiService)
        {
            // Resolution Independent Input Handling
            int winW, winH;
            SDL_Window* window = SDL_GL_GetCurrentWindow();
            SDL_GetWindowSize(window, &winW, &winH);
            
            int mx, my;
            Uint32 buttons = SDL_GetMouseState(&mx, &my);
            bool mouseDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT));
            
            static bool s_wasMouseDown = false;
            
            if (mouseDown && !s_wasMouseDown)
            {
                 glm::vec2 uiMouse = uiService->ScreenToUISpace((float)mx, (float)my, winW, winH);
                 uiService->HandleClick(uiMouse.x, uiMouse.y);
            }
            s_wasMouseDown = mouseDown;

            std::string action = uiService->GetLastAction();
            if (!action.empty())
            {
                if (action == "START_GAME")
                {
                    SwitchScene(GameState::Gameplay);
                }
                else if (action == "GEN_WORLD")
                {
                    auto dbService = ServiceLocator::Get().GetService<DatabaseService>();
                    auto worldService = std::dynamic_pointer_cast<WorldService>(ServiceLocator::Get().GetService<IWorldService>());
                    
                    if (dbService && worldService)
                    {
                        if (dbService->CreateNewWorld("NewWorld_" + std::to_string(rand() % 1000))) // Random generic name
                        {
                            worldService->GenerateInitialWorld(rand()); // Seed
                            
                            auto macroService = ServiceLocator::Get().GetService<IMacroService>();
                            if (macroService) macroService->GenerateSimulation(rand());

                            SwitchScene(GameState::DebugOverlay);
                        }
                    }
                }
                else if (action == "GEN_MACRO_WORLD")
                {
                    auto macroService = ServiceLocator::Get().GetService<IMacroService>();
                    auto logger = ServiceLocator::Get().GetService<ILoggerService>();
                    if (macroService) 
                    {
                        int seed = rand();
                        if (logger) logger->Log("Generating World with Seed: " + std::to_string(seed));
                        
                        macroService->GenerateSimulation(seed);
                        
                        if (uiService) 
                        {
                            uiService->SetElementText("macroWorldGeneration", "lbl_log_output", "Generated Macro World. Seed: " + std::to_string(seed));
                            uiService->SetElementTexture("macroWorldGeneration", "map_visualizer", macroService->GetMapTexture());
                        }
                    }
                }
                else if (action == "TOGGLE_POLITICAL") 
                {
                    auto macro = ServiceLocator::Get().GetService<IMacroService>();
                    if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Political);
                }
                else if (action == "TOGGLE_BIOMES")
                {
                    auto macro = ServiceLocator::Get().GetService<IMacroService>();
                    if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Biome);
                }
                else if (action == "TOGGLE_ECONOMY")
                {
                    auto macro = ServiceLocator::Get().GetService<IMacroService>();
                    if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Wealth);
                }
                else if (action == "TOGGLE_RUINATION")
                {
                    auto macro = ServiceLocator::Get().GetService<IMacroService>();
                    if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Ruination);
                }
                else if (action == "TOGGLE_SHADER_EDGE")
                {
                    // Toggle Edge shader or set style
                     if (m_currentStyle != RenderStyle::BORDERLANDS) m_currentStyle = RenderStyle::BORDERLANDS;
                     else m_currentStyle = RenderStyle::MINECRAFT; // Toggle off
                }
                else if (action == "TOGGLE_SHADER_CRT")
                {
                    // Toggle CRT/Moco style
                     if (m_currentStyle != RenderStyle::MOCO) m_currentStyle = RenderStyle::MOCO;
                     else m_currentStyle = RenderStyle::MINECRAFT; // Toggle off
                }
                
                uiService->ConsumeAction();
            }
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

    if (m_currentState == GameState::DebugOverlay)
    {
         renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
         renderer->Clear();
         
         // Disable Depth for Pixel Map to ensure overdraw works if needed (though it's 2D grid)
         renderer->SetDepthTest(false);
         RenderMacroMap(renderer.get());
         
          // Also render UI on top?
          if (uiService) 
          {
              renderer->SetDepthTest(false);
              // Simple render for debug UI
               int uiW = 1024, uiH = 768; // Logical size
               uiService->GetScreenSize(uiW, uiH);
               renderer->Begin(glm::ortho(0.0f, (float)uiW, (float)uiH, 0.0f, -1.0f, 1.0f));
               uiService->Render(renderer.get());
               renderer->End();
               renderer->SetDepthTest(true);
          }
         renderer->SetDepthTest(true);
         
         renderer->SwapBuffers();
         return;
    }

    // --- 1. World Pass (3D) ---
    if (renderWorld)
    {
        // ...
    }
    
    // ...

    // --- 3. UI Pass (2D) ---
    if (renderUI && uiService)
    {
         // Render UI on top of everything
         renderer->SetDepthTest(false);
         
         int uiW = 1280, uiH = 720;
         uiService->GetScreenSize(uiW, uiH);
         renderer->Begin(glm::ortho(0.0f, (float)uiW, (float)uiH, 0.0f, -1.0f, 1.0f));
         uiService->Render(renderer.get());
         renderer->End();
         
         renderer->SetDepthTest(true);
    }
    if (renderWorld)
    {
        // Safety check: if World is cleared, don't run this?
        // But WorldService still exists. ChunkManager might be empty.
        
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

        // Orthographic Projection for UI - FIXED to Logical 1024x768
        // User requested resolution independence. We force the projection to be logical 1024x768 always.
        // The Viewport is still window size, but projection maps 0..1024 to -1..1.
        glm::mat4 ortho = glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f, -1.0f, 1.0f);
        renderer->Begin(ortho);

        // Setup Shader for UI (reuse basic shader, disable effects)
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f)); // Identity View
        shaders->SetBool(m_basicShaderID, "u_celEnabled", false);
        shaders->SetBool(m_basicShaderID, "u_outlinesEnabled", false);

        renderer->SetDepthTest(false);
        if (uiService) uiService->Render(renderer.get()); 
        // Example: Debug/Crosshair could go here using renderer->DrawSprite(...)
        
        renderer->End();
        renderer->SetDepthTest(true);
    }

    if (usePost && renderWorld)
    {
        postProcess->Unbind(); 
        
        // Reset Viewport to Window Size
        int winW, winH;
        SDL_Window* window = SDL_GL_GetCurrentWindow();
        SDL_GetWindowSize(window, &winW, &winH);
        renderer->SetViewport(winW, winH);

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
         
         glDisable(GL_CULL_FACE); // Ensure quad is not culled
         
         // Reset Viewport to Window Size (PostProcess might have changed it)
         int winW, winH;
         SDL_Window* window = SDL_GL_GetCurrentWindow();
         SDL_GetWindowSize(window, &winW, &winH);
         renderer->SetViewport(winW, winH);

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

void GameLayer::RenderMacroMap(RenderService* renderer) {
    auto macroService = ServiceLocator::Get().GetService<IMacroService>();
    if (!macroService) return;

    unsigned int texID = macroService->GetMapTexture();
    if (texID == 0) return; // Not generated yet

    // Draw as a single large quad in the center
    // We want it to be 512x512 pixels on screen, centered.
    // Screen is 1280x720 (or whatever glViewport is).
    // Wait, we are in GameLayer::OnRender, and we set projection for UI pass?
    // No, RenderMacroMap is called in DebugOverlay state, where we cleared screen.
    // We should set up an Ortho projection that matches Window or Logical?
    // Let's use Logical 1024x768 for consistency with UI req.
    
    renderer->Begin(glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f, -1.0f, 1.0f));

    // Center of 1024x768 is 512, 384.
    // Map size 512x512.
    // Top-Left: 512 - 256 = 256. 384 - 256 = 128.
    
    Sprite mapSprite;
    mapSprite.Position = glm::vec2(512, 384);
    mapSprite.Size = glm::vec2(512, 512);
    mapSprite.TextureID = texID;
    mapSprite.Color = glm::vec4(1.0f);
    
    renderer->DrawSprite(mapSprite);
    
    // Draw Border (Optional, using solid color sprite behind or lines if supported)
    // ...

    renderer->End();
}

void GameLayer::SwitchScene(GameState newState)
{
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    auto worldService = ServiceLocator::Get().GetService<IWorldService>();

    m_currentState = newState;

    if (m_currentState == GameState::MainMenu)
    {
        if (uiService) uiService->SetScene("main_menu");
        if (worldService) worldService->ClearWorld();
        
        // Reset Camera?
        m_camera->target = glm::vec3(0.0f, 0.0f, 0.0f); 
    }
    else if (m_currentState == GameState::Gameplay)
    {
        if (uiService) uiService->SetScene("gameplay");
        
        // Generate World
        if (worldService) worldService->GenerateInitialWorld(1234); // Fixed seed for now
    }
    else if (m_currentState == GameState::DebugOverlay)
    {
        if (uiService) 
        {
            uiService->SetScene("macroWorldGeneration");
            auto macroService = ServiceLocator::Get().GetService<IMacroService>();
            if (macroService && macroService->GetMapTexture() != 0)
            {
                uiService->SetElementTexture("macroWorldGeneration", "map_visualizer", macroService->GetMapTexture());
            }
        }
    }
    else if (m_currentState == GameState::CharacterCreation)
    {
        if (uiService) uiService->SetScene("character_creation");
    }
    else if (m_currentState == GameState::PixelForge)
    {
        if (uiService) uiService->SetScene("pixel_forge");
    }
    else if (m_currentState == GameState::TradeInterface)
    {
        if (uiService) uiService->SetScene("trade_interface");
    }
    else if (m_currentState == GameState::DialogueWindow)
    {
        if (uiService) uiService->SetScene("dialogue_window");
    }
    else if (m_currentState == GameState::DeathSummary)
    {
        if (uiService) uiService->SetScene("death_summary");
    }
}
