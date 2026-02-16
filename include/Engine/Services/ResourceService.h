#pragma once
#include "Engine/Services/IResourceService.h"
#include <unordered_map>
#include <string>

class ResourceService : public IResourceService
{
public:
    ResourceService();
    ~ResourceService();

    void Init() override;
    void Clean() override;

    unsigned int GetTexture(const std::string& path) override;

private:
    std::unordered_map<std::string, unsigned int> m_textureCache;
};
