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
#include "Services/PostProcessService.h"
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
    camera->distance = 1500.0f;
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

    auto postProcess = std::make_shared<PostProcessService>(w, h);
    // postProcess->Init() is called by Register inside ServiceLocator
    ServiceLocator::Get().Register<PostProcessService>(postProcess);

    m_player = std::make_unique<Player>();
    m_player->Init();

    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    postEdgeShaderID = shaders->LoadShader("assets/shaders/post_edge.vert", "assets/shaders/post_edge.frag");

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
            std::string styleName = "BORDERLANDS";
            if (m_currentStyle == RenderStyle::MINECRAFT) styleName = "MINECRAFT";
            if (m_currentStyle == RenderStyle::MOCO) styleName = "MOCO";
            
            std::string title = "HD2D Engine | Style: " + styleName + " | Chunk: [" + std::to_string(cx) + ", " + std::to_string(cz) + "] | FPS: " + std::to_string((int)fps);
            SDL_SetWindowTitle(window, title.c_str());
        }

        // --- Input Handling for Styles ---
        if (inputService)
        {
            if (inputService->IsKeyDown(SDL_SCANCODE_1)) m_currentStyle = RenderStyle::MINECRAFT;
            if (inputService->IsKeyDown(SDL_SCANCODE_2)) m_currentStyle = RenderStyle::BORDERLANDS;
            if (inputService->IsKeyDown(SDL_SCANCODE_3)) m_currentStyle = RenderStyle::MOCO;
        }

        if (renderer)
        {
            auto postProcess = ServiceLocator::Get().GetService<PostProcessService>();
            bool usePost = (m_currentStyle != RenderStyle::MINECRAFT);

            // 1. Off-screen Rendering Pass (or Direct if Minecraft)
            if (usePost && postProcess) postProcess->Bind();
            else if (usePost) {} // No-op if postProcess null but requested
            else {
                 // Minecraft Style: Render directly to screen, so we need to clear default framebuffer
                 // But wait, renderer->Clear() below does that?
                 // Actually, if we bind 0 (default), then Clear() clears screen.
                 // If we bind FBO, Clear() clears FBO.
                 glBindFramebuffer(GL_FRAMEBUFFER, 0); // Ensure default if not using post
            }
            
            renderer->Clear();
            renderer->Begin();

            if (basicShaderID > 0)
            {
                shaders->UseShader(basicShaderID);

                // --- Cel Shading Uniforms ---
                // Rotating Light Source (1 rotation per 20 seconds)
                // Rotating Light Source (1 rotation per 20 seconds)
                float time = SDL_GetTicks() / 1000.0f;
                // Use DAY_CYCLE_SPEED alias as requested
                float angle = time * (glm::two_pi<float>() / GameConfig::DAY_CYCLE_SPEED);
                float lightX = sin(angle);
                float lightZ = cos(angle);
                glm::vec3 lightDir = glm::normalize(glm::vec3(lightX, 1.0f, lightZ));

                shaders->SetVec3(basicShaderID, "u_lightDir", lightDir);
                
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

                shaders->SetInt(basicShaderID, "u_lightBands", lightBands);
                shaders->SetFloat(basicShaderID, "u_rimStrength", rimStrength);
                shaders->SetVec3(basicShaderID, "u_viewPos", camera->GetPosition());

                // Keep config values for these, or override? Prompt implies we override lightBands but maybe keep thickness?
                // Prompt: "Uniforms: u_lightBands = ..."
                // Let's use the switch values for bands and rim.
                // Outline thickness wasn't specified to change per style in prompt (only color in post).
                // Actually, Minecraft needs outlines DISABLED?
                // Prompt: "Style 1 (Minecraft): Disable Post-Processing... u_lightBands = 256... u_rimStrength = 0.0".
                // Does NOT say disable cel shading or outlines in basic shader, just post process.
                // But usually Minecraft style implies standard look. 
                // Wait, "Disable Post-Processing" means no edge detection.
                // But basicShader might still do cel shading if u_celEnabled is true.
                // I will keep Global Config for enabled/disabled for now as prompt didn't say to toggle u_celEnabled.
                
                shaders->SetFloat(basicShaderID, "u_outlineThickness", GameConfig::CelShading::OUTLINE_WIDTH);
                shaders->SetFloat(basicShaderID, "u_ambientStrength", GameConfig::CelShading::AMBIENT_STRENGTH);
                shaders->SetBool(basicShaderID, "u_celEnabled", GameConfig::CelShading::ENABLED);
                shaders->SetBool(basicShaderID, "u_outlinesEnabled", GameConfig::CelShading::OUTLINES_ENABLED);
                // ---------------------------

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

            // 2. Post-Processing Pass
            if (usePost && postProcess)
            {
                postProcess->Unbind(); // Switch back to default framebuffer
                renderer->Clear(); // Clear the screen (optional, but good practice)
                
                // Disable Depth Testing for Quad Rendering (usually)
                glDisable(GL_DEPTH_TEST);
                
                if (postEdgeShaderID > 0)
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
                            depthThresh = 0.02f; // Default
                            outlineColor = glm::vec4(0.2f, 0.1f, 0.1f, 1.0f);
                            break;
                        default: break;
                    }

                    postProcess->RenderRect(shaders, postEdgeShaderID, normalThresh, depthThresh, outlineColor);
                }
                
                glEnable(GL_DEPTH_TEST); // Re-enable for next frame's 3D rendering
            }
            
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