#pragma once
#include "Engine/Services/IResourceService.h"
#include <unordered_map>
#include <string>

class ResourceService : public IResourceService
{
public:
    ResourceService();
    ~ResourceService();

protected:
    void OnInitialize() override;
public:
    void Clean() override;

    unsigned int GetTexture(const std::string& path) override;

private:
    std::unordered_map<std::string, unsigned int> m_textureCache;
};
