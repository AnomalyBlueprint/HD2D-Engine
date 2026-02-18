#include "Game/Scenes/MainMenuScene.h"
#include "Game/Scenes/GameplayScene.h" // Needed to switch to gameplay
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/UIService.h"
#include "Engine/Services/InputService.h"
#include "Engine/Services/MacroService.h"
#include "Engine/Services/DatabaseService.h"
#include "Engine/Services/WorldService.h"
#include "Engine/Services/SceneService.h"
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/ILoggerService.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>

MainMenuScene::MainMenuScene() {}

MainMenuScene::~MainMenuScene() {}

void MainMenuScene::OnEnter() {
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    if (uiService) {
        uiService->SetScene("main_menu");
    }
    
    // Clear any previous world data to free memory
    auto worldService = ServiceLocator::Get().GetService<IWorldService>();
    if (worldService) {
        worldService->ClearWorld();
    }
    
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    if (shaders) {
        m_basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    }

    m_debugOverlay = false;
}

void MainMenuScene::OnExit() {
    // Cleanup if needed
}

void MainMenuScene::OnUpdate(float deltaTime) {
    (void)deltaTime;
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    auto inputService = ServiceLocator::Get().GetService<IInputService>();
    
    // Handle specific keys for this scene
    // Handle specific keys for this scene
    if (inputService) {
        if (inputService->IsKeyPressed(SDL_SCANCODE_F1)) {
             auto logger = ServiceLocator::Get().GetService<ILoggerService>();
             if (logger) logger->Log("STATUS: Main Menu Scene is Active");
             
             SDL_Window* window = SDL_GL_GetCurrentWindow();
             if (window) SDL_SetWindowTitle(window, "Main Menu Scene");
        }
    }

    // UI Interactions
    if (uiService) {
        int winW, winH;
        SDL_Window* window = SDL_GL_GetCurrentWindow();
        SDL_GetWindowSize(window, &winW, &winH);
        
        int mx, my;
        Uint32 buttons = SDL_GetMouseState(&mx, &my);
        bool mouseDown = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT));
        
        static bool s_wasMouseDown = false;
        
        if (mouseDown && !s_wasMouseDown) {
             glm::vec2 uiMouse = uiService->ScreenToUISpace((float)mx, (float)my, winW, winH);
             uiService->HandleClick(uiMouse.x, uiMouse.y);
        }
        s_wasMouseDown = mouseDown;

        // Process Actions
        std::string action = uiService->GetLastAction();
        if (!action.empty()) {
            if (action == "START_GAME") {
                auto sceneService = ServiceLocator::Get().GetService<SceneService>();
                if (sceneService) sceneService->LoadScene(new GameplayScene());
            }
            else if (action == "GEN_WORLD") {
                auto dbService = ServiceLocator::Get().GetService<DatabaseService>();
                auto worldService = std::dynamic_pointer_cast<WorldService>(ServiceLocator::Get().GetService<IWorldService>());
                
                if (dbService && worldService) {
                    if (dbService->CreateNewWorld("NewWorld_" + std::to_string(rand() % 1000))) {
                        worldService->GenerateInitialWorld(rand());
                        
                        auto macroService = ServiceLocator::Get().GetService<IMacroService>();
                        if (macroService) macroService->GenerateSimulation(rand());
                        
                        auto logger = ServiceLocator::Get().GetService<ILoggerService>();
                        if (logger) logger->Log("SUCCESS: World Generated and Scene Switched to Macro View.");

                        // Switch to Debug Overlay
                        m_debugOverlay = true;
                        uiService->SetScene("macroWorldGeneration");
                        if (macroService) {
                            uiService->SetElementTexture("macroWorldGeneration", "map_visualizer", macroService->GetMapTexture());
                            UpdateMapView();
                        }
                    }
                }
            }
            else if (action == "GEN_MACRO_WORLD") {
                auto macroService = ServiceLocator::Get().GetService<IMacroService>();
                auto logger = ServiceLocator::Get().GetService<ILoggerService>();
                if (macroService) {
                    int seed = rand();
                    if (logger) logger->Log("Generating World with Seed: " + std::to_string(seed));
                    
                    macroService->GenerateSimulation(seed);
                    
                    if (uiService) {
                        uiService->SetElementText("macroWorldGeneration", "lbl_log_output", "Generated Macro World. Seed: " + std::to_string(seed));
                        uiService->SetElementTexture("macroWorldGeneration", "map_visualizer", macroService->GetMapTexture());
                    }
                }
            }
            else if (action == "TOGGLE_POLITICAL") {
                auto macro = ServiceLocator::Get().GetService<IMacroService>();
                if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Political);
            }
            else if (action == "TOGGLE_BIOMES") {
                auto macro = ServiceLocator::Get().GetService<IMacroService>();
                if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Biome);
            }
            else if (action == "TOGGLE_ECONOMY") {
                auto macro = ServiceLocator::Get().GetService<IMacroService>();
                if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Wealth);
            }
            else if (action == "TOGGLE_RUINATION") {
                auto macro = ServiceLocator::Get().GetService<IMacroService>();
                if(macro) macro->SetViewMode(IMacroService::MacroViewMode::Ruination);
            }
            else if (action == "MAP_ZOOM_IN") {
                m_mapZoom *= 1.2f;
                if (m_mapZoom > 10.0f) m_mapZoom = 10.0f;
                UpdateMapView();
            }
            else if (action == "MAP_ZOOM_OUT") {
                m_mapZoom /= 1.2f;
                if (m_mapZoom < 1.0f) m_mapZoom = 1.0f;
                UpdateMapView();
            }
            else if (action == "MAP_PAN_UP") {
                m_mapOffset.y -= 0.1f / m_mapZoom;
                UpdateMapView();
            }
            else if (action == "MAP_PAN_DOWN") {
                m_mapOffset.y += 0.1f / m_mapZoom;
                UpdateMapView();
            }
            else if (action == "MAP_PAN_LEFT") {
                m_mapOffset.x -= 0.1f / m_mapZoom;
                UpdateMapView();
            }
            else if (action == "MAP_PAN_RIGHT") {
                m_mapOffset.x += 0.1f / m_mapZoom;
                UpdateMapView();
            }
            else if (action == "MAP_RESET") {
                m_mapZoom = 1.0f;
                m_mapOffset = glm::vec2(0.0f);
                UpdateMapView();
            }
            else if (action == "BACK_TO_MENU") {
                m_debugOverlay = false;
                if (uiService) uiService->SetScene("main_menu");
            }
            
            uiService->ConsumeAction();
        }
    }
}

void MainMenuScene::UpdateMapView() {
    float visiblePortion = 1.0f / m_mapZoom;
    float halfVis = visiblePortion / 2.0f;
    
    // Clamp offset to keep view within bounds
    float maxOffset = (1.0f - visiblePortion) / 2.0f;
    if (maxOffset < 0) maxOffset = 0;
    
    m_mapOffset.x = glm::clamp(m_mapOffset.x, -maxOffset, maxOffset);
    m_mapOffset.y = glm::clamp(m_mapOffset.y, -maxOffset, maxOffset);

    glm::vec2 center = glm::vec2(0.5f) + m_mapOffset;
    glm::vec2 minUV = center - glm::vec2(halfVis);
    glm::vec2 maxUV = center + glm::vec2(halfVis);
    
    auto ui = ServiceLocator::Get().GetService<IUIService>();
    if (ui) {
        ui->SetElementUVs("macroWorldGeneration", "map_visualizer", minUV, maxUV);
    }
}

void MainMenuScene::OnRender() {
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    
    if (!renderer || !uiService) return;

    renderer->SetClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    renderer->Clear();
    
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    if (shaders && m_basicShaderID > 0) {
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f));
        shaders->SetBool(m_basicShaderID, "u_celEnabled", false); // Disable 3D effects for UI
        shaders->SetBool(m_basicShaderID, "u_outlinesEnabled", false);
    }
    
    // Disable Culling for 2D UI (Prevent winding order issues with Y-down Ortho)
    glDisable(GL_CULL_FACE);
    // Disable Depth Test for UI to ensure it draws on top and doesn't Z-fight
    glDisable(GL_DEPTH_TEST);

    // Render UI directly
    // Assuming UI Service handles logic for what is visible based on SetScene call in OnEnter/Update
    int uiW = 1024, uiH = 768; // Logical size
    uiService->GetScreenSize(uiW, uiH);
    
    // Ortho for UI
    renderer->Begin(glm::ortho(0.0f, (float)uiW, (float)uiH, 0.0f, -1.0f, 1.0f));
    
    // If debug overlay and macro map texture exists, we might want to render it?
    // Actually, handling it via UI Element Texture update in Update() is cleaner 
    // and handled by UIService::Render().
    
    uiService->Render(renderer.get());
    renderer->End();
    
    glEnable(GL_CULL_FACE); // Restore for other scenes/passes
    glEnable(GL_DEPTH_TEST);
    
    renderer->SwapBuffers();
}

void MainMenuScene::OnEvent(SDL_Event& e) {
    (void)e; // Silence unused warning
    // Input Service handles most things, but if we need specific event handling
}
