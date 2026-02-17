#include "Engine/Services/ResourceService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/ILoggerService.h"
#include "Engine/Services/PathRegistryService.h"
#include "Engine/Services/UIPathRepository.h"
#include "Engine/Services/FontPathRepository.h"
#include <iostream>

ResourceService::ResourceService()
{
}

ResourceService::~ResourceService()
{
    Clean();
}

void ResourceService::OnInitialize()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    if(log) log->Log("Resource Service Initialized.");

    // Register Path Repositories
    auto pathRegistry = std::make_shared<PathRegistryService>(); // Actually Engine or GameLayer owns this... 
    // Wait, PathRegistryService is a service itself.
    // Let's assume PathRegistryService is already registered by Engine.cpp.
    // IF NOT, we should check availability. 
    
    // Actually, based on previous code, ResourceService didn't own PathRegistry.
    // The user requested: "Register these new repositories in ResourceService::OnInitialize."
    // But usually PathRegistry holds Repos. 
    // Let's try to get PathRegistry and add repos to it.
    
    auto registry = ServiceLocator::Get().GetService<PathRegistryService>();
    if (registry)
    {
        registry->RegisterRepository<UIPathRepository>(std::make_shared<UIPathRepository>());
        registry->RegisterRepository<FontPathRepository>(std::make_shared<FontPathRepository>());
        
        // Initialize them
        registry->GetRepository<UIPathRepository>()->OnInitialize();
        registry->GetRepository<FontPathRepository>()->OnInitialize();
    }
    else
    {
        if(log) log->LogWarning("PathRegistryService not found in ResourceService::OnInitialize");
    }
}

void ResourceService::Clean()
{
    m_textureCache.clear();
}

unsigned int ResourceService::GetTexture(const std::string& path)
{
    // 1. Check Cache
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end())
    {
        return it->second;
    }

    // 2. Load New (via RenderService)
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    if (!renderer)
    {
        // Fallback or Error
        std::cerr << "[ResourceService] Critical: RenderService not found!" << std::endl;
        return 0;
    }

    unsigned int textureID = renderer->LoadTexture(path);

    // 3. Cache it
    if (textureID > 0)
    {
        m_textureCache[path] = textureID;
    }

    return textureID;
}
