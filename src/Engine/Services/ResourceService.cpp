#include "Engine/Services/ResourceService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/ILoggerService.h"
#include <iostream>

ResourceService::ResourceService()
{
}

ResourceService::~ResourceService()
{
    Clean();
}

void ResourceService::Init()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    log->Log("Resource Service Initialized.");
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
