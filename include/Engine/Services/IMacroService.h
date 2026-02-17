#pragma once
#include "Engine/Services/IService.h"
#include <vector>

struct MacroTile {
    uint8_t BiomeID;
    uint8_t Height;
    uint8_t PoliticalID;
    uint8_t Wealth;
    uint8_t Ruination;
};

class IMacroService : public IService {
public:
    virtual ~IMacroService() = default;

    enum class MacroViewMode { Political, Biome, Wealth, Ruination, Height };

    virtual void GenerateSimulation(int seed) = 0;
    virtual unsigned int GetMapTexture() = 0;
    virtual void SetViewMode(MacroViewMode mode) = 0;
    virtual const std::vector<MacroTile>& GetMacroGrid() const = 0;
};
