#pragma once
#include "Services/IService.h"
#include "Data/KenneyIDs.h"

/// <summary>
/// Interface for managing block definitions and texture lookups.
/// Decouples the Chunk logic from specific Texture IDs.
/// </summary>
class IBlockRegistryService : public IService {
public:
    virtual ~IBlockRegistryService() = default;

    /// <summary>
    /// Gets the texture ID for a specific block and face.
    /// </summary>
    /// <param name="blockID">The byte ID of the block (1=Dirt, 2=Grass, etc).</param>
    /// <param name="faceID">The face index (0=Front, 1=Back, 2=Right, 3=Left, 4=Top, 5=Bottom).</param>
    /// <returns>The KenneyIDs enum for the texture.</returns>
    virtual KenneyIDs GetTextureID(int blockID, int faceID) = 0;
};
