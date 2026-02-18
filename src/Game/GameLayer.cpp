#include "Game/GameLayer.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/SceneService.h"
#include "Game/Scenes/MainMenuScene.h"
#include "Engine/Services/WorldService.h"
#include "Engine/Services/MacroService.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/BlockRegistryService.h"
#include "Engine/Services/DatabaseService.h"
#include "Engine/Services/ChunkManager.h"
#include "Engine/Services/UIService.h"
#include "Engine/Services/PostProcessService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/KenneyPathRepository.h"
#include "Engine/Services/ILoggerService.h"
#include <SDL.h>

GameLayer::GameLayer() {}

GameLayer::~GameLayer() {}

void GameLayer::OnAttach()
{
    // Initialize Game Specific Services
    // Note: Some of these might be better in Engine::Init if they are core, 
    // but sticking to original design where GameLayer sets up Game Services.
    
    auto worldService = std::make_shared<WorldService>();
    ServiceLocator::Get().Register<IWorldService>(worldService);

    auto macroService = std::make_shared<MacroService>();
    ServiceLocator::Get().Register<IMacroService>(macroService);

    auto atlasService = std::make_shared<TextureAtlasService>();
    auto pathRegistry = ServiceLocator::Get().GetService<PathRegistryService>();
    auto kenneyRepo = pathRegistry->GetRepository<KenneyPathRepository>(); 
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

    int w = 1280, h = 720; 
    SDL_Window* window = SDL_GL_GetCurrentWindow();
    if (window) SDL_GetWindowSize(window, &w, &h);
    
    auto postProcess = std::make_shared<PostProcessService>(w, h);
    ServiceLocator::Get().Register<PostProcessService>(postProcess);
    
    // Bootstrap Scene System
    // User requested "In GameLayer::OnAttach, initialize SceneService and immediately load new MainMenuScene()"
    // SceneService was registered in Engine::Init, so we just get it.
    auto sceneService = ServiceLocator::Get().GetService<SceneService>();
    if (sceneService) {
        sceneService->LoadScene(new MainMenuScene());
    }
}

void GameLayer::OnDetach()
{
    // Cleanup
}

void GameLayer::OnUpdate(float deltaTime)
{
    auto sceneService = ServiceLocator::Get().GetService<SceneService>();
    if (sceneService) sceneService->Update(deltaTime);
}

void GameLayer::OnRender()
{
    auto sceneService = ServiceLocator::Get().GetService<SceneService>();
    if (sceneService) sceneService->Render();
}

void GameLayer::OnEvent(SDL_Event& e)
{
    auto sceneService = ServiceLocator::Get().GetService<SceneService>();
    if (sceneService) sceneService->OnEvent(e);
}
