#pragma once
#include "Services/IService.h"
#include "World/Chunk.h"

class IWorldService : public IService
{
public:
    virtual void GenerateChunk(Chunk* chunk, int worldX, int worldY) = 0;
};
