#pragma once
#include "Engine/Core/IService.h"
#include "Engine/Data/KenneyIDs.h"
#include <cstdint>

/// <summary>
/// Optimized Block Definition for Data-Driven Rendering
/// </summary>
struct BlockDef {
    KenneyIDs textureTop;
    KenneyIDs textureSide;
    KenneyIDs textureBottom;
    bool isTransparent;
    bool isCollidable;
};

/// <summary>
/// Interface for managing block definitions.
/// </summary>
class IBlockRegistryService : public IService {
public:
    virtual ~IBlockRegistryService() = default;

protected:
    virtual void OnInitialize() override = 0;
public:
    virtual void Clean() override = 0;

    /// <summary>
    /// Gets the full definition for a block.
    /// </summary>
    virtual const BlockDef& GetBlock(uint8_t id) = 0;
};
