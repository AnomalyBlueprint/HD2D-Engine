#include "Game/Scenes/GameplayScene.h"
#include "Game/Scenes/MainMenuScene.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/IInputService.h"
#include "Engine/Services/IWorldService.h"
#include "Engine/Services/ChunkManager.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/PostProcessService.h"
#include "Engine/UI/UIService.h"
#include "Engine/Services/SceneService.h"
#include "Game/GameConfig.h"
#include "Engine/Services/ILoggerService.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

GameplayScene::GameplayScene() {}

GameplayScene::~GameplayScene() {}

void GameplayScene::OnEnter() {
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    if (uiService) uiService->SetScene("gameplay");

    auto worldService = ServiceLocator::Get().GetService<IWorldService>();
    // Ensure world is generated if not already? Scene transition might have triggered it.
    // If we came from MainMenu -> New Game, WorldService is populated.
    // If we want to support loading, we'd do it here.
    
    // Camera & Player
    int w = 1280, h = 720; // TODO: Get from Window
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    if (window) SDL_GetWindowSize(window, &w, &h);

    float aspect = (float)w / (float)h;
    m_camera = std::make_shared<Camera>(aspect);
    m_camera->target = glm::vec3(0.0f, 0.0f, 0.0f); 
    m_camera->distance = 1500.0f;
    m_camera->orbitAngle = 0.0f;

    m_player = std::make_unique<Player>();
    m_player->Init();

    // Shaders & Textures
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    m_basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    m_postEdgeShaderID = shaders->LoadShader("assets/shaders/post_edge.vert", "assets/shaders/post_edge.frag"); // Cached if already loaded

    auto atlasService = ServiceLocator::Get().GetService<TextureAtlasService>();
    if (atlasService) {
        m_textureID = atlasService->GetTextureID();
    }
}

void GameplayScene::OnExit() {
    auto worldService = ServiceLocator::Get().GetService<IWorldService>();
    if (worldService) {
        worldService->ClearWorld(); // Free chunks
    }
}

void GameplayScene::OnUpdate(float deltaTime) {
    auto inputService = ServiceLocator::Get().GetService<IInputService>();
    auto uiService = ServiceLocator::Get().GetService<IUIService>();
    
    if (inputService) {
        if (inputService->IsKeyDown(SDL_SCANCODE_ESCAPE)) {
            // Switch to Main Menu
             auto sceneService = ServiceLocator::Get().GetService<SceneService>();
             if (sceneService) sceneService->LoadScene(new MainMenuScene());
             return; 
        }

        // Scene Switching Keys (Debug)
        // Scene Switching Keys (Debug)
        if (inputService->IsKeyPressed(SDL_SCANCODE_F1)) {
             auto logger = ServiceLocator::Get().GetService<ILoggerService>();
             if (logger) logger->Log("STATUS: Gameplay Scene is Active");
             
             SDL_Window* window = SDL_GL_GetCurrentWindow();
             if (window) SDL_SetWindowTitle(window, "Gameplay Scene");
        }
        
        // Update Player & Camera
        m_player->Update(deltaTime, inputService.get());
        m_camera->Update(m_player->GetPosition(), deltaTime, inputService.get());
    }
    
    // UI Logic (Handling Clicks)
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
        
        std::string action = uiService->GetLastAction();
        if (!action.empty()) {
            if (action == "TOGGLE_SHADER_EDGE") {
                if (m_currentStyle != RenderStyle::BORDERLANDS) m_currentStyle = RenderStyle::BORDERLANDS;
                else m_currentStyle = RenderStyle::MINECRAFT; 
            }
            else if (action == "TOGGLE_MOCO") {
                if (m_currentStyle != RenderStyle::MOCO) m_currentStyle = RenderStyle::MOCO;
                else m_currentStyle = RenderStyle::MINECRAFT; 
            }
            
            uiService->ConsumeAction();
        }
    }
    
    // Title Update
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

void GameplayScene::OnRender() {
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();
    auto postProcess = ServiceLocator::Get().GetService<PostProcessService>();
    auto uiService = ServiceLocator::Get().GetService<IUIService>();

    if (!renderer || !shaders) return;

    bool usePost = postProcess != nullptr;

    // --- 1. World Pass ---
    if (usePost) postProcess->SetTargetLayer("World");
    else glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderer->SetClearColor(0.39f, 0.58f, 0.93f, 1.0f);
    renderer->Clear();
    
    glm::mat4 projection = m_camera->GetProjectionMatrix();
    glm::mat4 view = m_camera->GetViewMatrix();
    
    renderer->Begin(projection);
    
    if (m_basicShaderID > 0) {
        shaders->UseShader(m_basicShaderID);
        
        // Uniforms
        float time = SDL_GetTicks() / 1000.0f;
        float angle = time * (glm::two_pi<float>() / GameConfig::DAY_CYCLE_SPEED);
        float lightX = sin(angle);
        float lightZ = cos(angle);
        glm::vec3 lightDir = glm::normalize(glm::vec3(lightX, 1.0f, lightZ));
        shaders->SetVec3(m_basicShaderID, "u_lightDir", lightDir);
        
        int lightBands = 256;
        float rimStrength = 0.0f;
        switch (m_currentStyle) {
            case RenderStyle::MINECRAFT: lightBands = 256; rimStrength = 0.0f; break;
            case RenderStyle::BORDERLANDS: lightBands = 2; rimStrength = 0.1f; break;
            case RenderStyle::MOCO: lightBands = 4; rimStrength = 0.6f; break;
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
        
        if (chunkManager) {
            chunkManager->Update(m_player->GetPosition());
            renderer->UseTexture(m_textureID);
            chunkManager->Render(renderer.get(), shaders.get());
        }
        
        if (m_player) {
            renderer->UseTexture(0);
            m_player->Render(renderer.get(), shaders.get(), m_basicShaderID);
        }
    }
    renderer->End();
    
    // --- 2. UI Pass (To Texture) ---
    if (usePost) {
        postProcess->SetTargetLayer("UI");
        renderer->SetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        renderer->Clear();
        
        glm::mat4 ortho = glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f, -1.0f, 1.0f);
        renderer->Begin(ortho);
        
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f));
        shaders->SetBool(m_basicShaderID, "u_celEnabled", false);
        shaders->SetBool(m_basicShaderID, "u_outlinesEnabled", false);
        
        renderer->SetDepthTest(false);
        if (uiService) uiService->Render(renderer.get());
        renderer->End();
        renderer->SetDepthTest(true);
    }
    
    // --- 3. Composite (World + Post + UI) ---
    if (usePost) {
        postProcess->Unbind();
        
        int winW, winH;
        SDL_Window* window = SDL_GL_GetCurrentWindow();
        SDL_GetWindowSize(window, &winW, &winH);
        renderer->SetViewport(winW, winH);
        
        renderer->SetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        renderer->Clear();
        
        glDisable(GL_DEPTH_TEST);
        
        // Draw World with Edge Detection
        if (m_postEdgeShaderID > 0) {
             float normalThresh = 0.4f;
             float depthThresh = 0.02f;
             glm::vec4 outlineColor(0,0,0,1);
             
             if (m_currentStyle == RenderStyle::MOCO) {
                 normalThresh = 0.6f;
                 outlineColor = glm::vec4(0.2f, 0.1f, 0.1f, 1.0f);
             }
             
             postProcess->RenderLayerWithEdges("World", shaders, m_postEdgeShaderID, normalThresh, depthThresh, outlineColor);
        }
        
        // Draw UI Overlay
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        shaders->UseShader(m_basicShaderID);
        shaders->SetMat4(m_basicShaderID, "view", glm::mat4(1.0f));
        shaders->SetMat4(m_basicShaderID, "projection", glm::mat4(1.0f));
        
        renderer->Begin(glm::mat4(1.0f));
        
        Sprite uiFSQ;
        uiFSQ.Position = glm::vec2(0,0);
        uiFSQ.Size = glm::vec2(2,2);
        uiFSQ.TextureID = postProcess->GetTexture("UI");
        uiFSQ.Color = glm::vec4(1.0f);
        
        renderer->DrawSprite(uiFSQ);
        renderer->End();
        
        glEnable(GL_DEPTH_TEST);
    }
    
    renderer->SwapBuffers();
}

void GameplayScene::OnEvent(SDL_Event& e) {
    
}
