#pragma once
#include "Engine/Services/IWorldService.h"
#include "vendor/FastNoiseLite.h"

/// <summary>
/// Implementation of IWorldService using FastNoiseLite.
/// Generates terraced terrain.
/// </summary>
#include <vector>
#include <string>

struct MacroTile {
    uint8_t BiomeID;
    uint32_t SeedOffset;
    uint8_t Ruination;
    uint8_t Wealth;
};

struct WorldGenConfig {
    float NoiseFrequency = 0.01f;
    int Octaves = 4;
    int SeaLevel = 5;
};

class WorldService : public IWorldService
{
public:
    WorldService();
    ~WorldService() = default;
    // IWorldService
    void GenerateInitialWorld(int seed) override;
    void ClearWorld() override;

    // Macro Grid
    void GenerateMacroGrid();
    const std::vector<MacroTile>& GetMacroGrid() const { return m_macroGrid; }
    void LoadWorldGenConfig(const std::string& path);

protected:
    void OnInitialize() override;
public:
    void Clean() override;
    void GenerateChunk(Chunk* chunk, int worldX, int worldY) override;
    uint8_t GetBlockAt(int globalX, int globalY, int globalZ) override;

private:
    FastNoiseLite m_noise;
    std::vector<MacroTile> m_macroGrid; // 256x256
    WorldGenConfig m_config;
    int m_globalSeed = 0;
};
