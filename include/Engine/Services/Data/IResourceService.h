#pragma once
#include "Engine/Core/IService.h"
#include <string>

/// <summary>
/// Interface for loading and caching resources (textures).
/// </summary>
class IResourceService : public IService
{
public:
    virtual ~IResourceService() = default;

    /// <summary>
    /// Retrieves the OpenGL Texture ID for the given file path.
    /// Loads the texture if it's not already cached.
    /// </summary>
    /// <param name="path">Absolute or relative path to the image file.</param>
    /// <returns>OpenGL Texture ID.</returns>
    virtual unsigned int GetTexture(const std::string& path) = 0;
};
