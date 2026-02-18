#pragma once
#include "Engine/Services/World/IMacroService.h"
#include <vendor/FastNoiseLite.h>
#include <vector>

class MacroService : public IMacroService {
public:
    MacroService();
    ~MacroService();

    void GenerateSimulation(int seed) override;
    unsigned int GetMapTexture() override;
    void SetViewMode(MacroViewMode mode) override;
    const std::vector<MacroTile>& GetMacroGrid() const override { return m_grid; }

    void Clean() override;

protected:
    void OnInitialize() override;

private:
    std::vector<MacroTile> m_grid; // Flat array for 256x256
    unsigned int m_textureID = 0;
    MacroViewMode m_currentMode = MacroViewMode::Biome;
    
    // Helper to map biome data to color
    // Helper to map biome data to color
    void UpdateTexture();
    void RegenerateTexture(); // New method for RGBA generation
};
