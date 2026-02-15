#pragma once
#include "Services/IWorldService.h"
#include "vendor/FastNoiseLite.h"

class WorldService : public IWorldService
{
public:
    WorldService();
    ~WorldService() = default;

    void Init() override;
    void Clean() override;
    void GenerateChunk(Chunk* chunk, int worldX, int worldY) override;

private:
    FastNoiseLite m_noise;
};
