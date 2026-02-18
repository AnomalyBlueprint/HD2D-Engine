#pragma once
#include "Engine/Core/IService.h"
#include <string>

/// <summary>
/// Interface for retrieving asset paths by ID.
/// </summary>
class IPathRepository : public IService
{
public:
    virtual ~IPathRepository() = default;
    
    /// <summary>
    /// Gets the file path for the specified asset ID.
    /// </summary>
    virtual std::string GetPath(int assetID) = 0;
};
