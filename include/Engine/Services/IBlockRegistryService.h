#pragma once
#include "Engine/Services/IService.h"
#include "Engine/Data/KenneyIDs.h"
#include <string>
#include <cstdint>

/// <summary>
/// Interface for managing block definitions and texture lookups.
/// Decouples the Chunk logic from specific Texture IDs.
/// </summary>
struct BlockDefinition {
    uint8_t ID;
    std::string Name;
    bool IsTransparent;
    bool IsCollidable; // For physics
    KenneyIDs TextureTop;
    KenneyIDs TextureSide;
    KenneyIDs TextureBottom;
};

/// <summary>
/// Interface for managing block definitions and texture lookups.
/// Decouples the Chunk logic from specific Texture IDs.
/// </summary>
class IBlockRegistryService : public IService {
public:
    virtual ~IBlockRegistryService() = default;

    virtual void Init() override = 0;
    virtual void Clean() override = 0;

    /// <summary>
    /// Gets the texture ID for a specific block and face.
    /// </summary>
    virtual KenneyIDs GetTextureID(int blockID, int faceID) = 0;

    /// <summary>
    /// Gets the full definition for a block.
    /// </summary>
    virtual const BlockDefinition& GetBlockDef(int blockID) = 0;
};
