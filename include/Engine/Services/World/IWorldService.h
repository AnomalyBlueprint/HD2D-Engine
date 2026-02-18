#pragma once
#include "Engine/Services/IService.h"
#include "Game/World/Chunk.h"

/// <summary>
/// Interface for World Generation and Block Data retrieval.
/// </summary>
class IWorldService : public IService
{
public:
    /// <summary>
    /// Generates the initial world state with a specific seed.
    /// </summary>
    virtual void GenerateInitialWorld(int seed) = 0;

    /// <summary>
    /// Clears the current world data.
    /// </summary>
    virtual void ClearWorld() = 0;

    /// <summary>
    /// Generates terrain for a specific chunk column.
    /// </summary>
    virtual void GenerateChunk(Chunk* chunk, int worldX, int worldY) = 0;

    /// <summary>
    /// Retrieves the block ID at specific global coordinates.
    /// Used for seamless neighbor checking.
    /// </summary>
    virtual uint8_t GetBlockAt(int globalX, int globalY, int globalZ) = 0;
};
