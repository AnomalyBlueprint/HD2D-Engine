#pragma once
#include "Services/IWorldService.h"
#include "vendor/FastNoiseLite.h"

/// <summary>
/// Implementation of IWorldService using FastNoiseLite.
/// Generates terraced terrain.
/// </summary>
class WorldService : public IWorldService
{
public:
    WorldService();
    ~WorldService() = default;

    void Init() override;
    void Clean() override;
    void GenerateChunk(Chunk* chunk, int worldX, int worldY) override;
    uint8_t GetBlockAt(int globalX, int globalY, int globalZ) override;

private:
    FastNoiseLite m_noise;
};
