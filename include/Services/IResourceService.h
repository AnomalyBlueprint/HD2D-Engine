#pragma once
#include "Services/IService.h"
#include <string>

class IResourceService : public IService
{
public:
    virtual ~IResourceService() = default;

    // Returns the OpenGL Texture ID for the given path.
    // Loads it if not already loaded.
    virtual unsigned int GetTexture(const std::string& path) = 0;
};
