#pragma once
#include "Services/IService.h"
#include "World/Chunk.h"

/// <summary>
/// Interface for World Generation and Block Data retrieval.
/// </summary>
class IWorldService : public IService
{
public:
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
