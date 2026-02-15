#pragma once
#include "Services/IService.h"
#include "World/Chunk.h"

class IWorldService : public IService
{
public:
    virtual void GenerateChunk(Chunk* chunk, int worldX, int worldY) = 0;
    virtual uint8_t GetBlockAt(int globalX, int globalY, int globalZ) = 0;
};
