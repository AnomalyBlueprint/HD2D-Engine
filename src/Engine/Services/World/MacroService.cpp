#include "Engine/Services/World/MacroService.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/ILoggerService.h"
#include <GL/glew.h>
#include <iostream>
#include "Engine/Core/BitmapFont.h"

// Constants
constexpr int PIPELINE_WIDTH = 256;
constexpr int PIPELINE_HEIGHT = 256;

MacroService::MacroService() {
    m_grid.resize(PIPELINE_WIDTH * PIPELINE_HEIGHT);
}

MacroService::~MacroService() {
    Clean();
}

void MacroService::OnInitialize() {
    // Auto-generate on init with random seed? Or wait for explicit call?
    // Let's wait for explicit call from GameLayer (GEN_WORLD action)
}

void MacroService::Clean() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    m_grid.clear();
}

void MacroService::GenerateSimulation(int seed) {
    auto logger = ServiceLocator::Get().GetService<ILoggerService>();
    if (logger) logger->Log("Generating Macro World with Seed: " + std::to_string(seed));

    FastNoiseLite noise;
    noise.SetSeed(seed);
    noise.SetNoiseType(FastNoiseLite::NoiseType::Perlin);
    noise.SetFrequency(0.02f); // Adjust for scale

    FastNoiseLite heightNoise;
    heightNoise.SetSeed(seed + 1);
    heightNoise.SetNoiseType(FastNoiseLite::NoiseType::Perlin);
    heightNoise.SetFrequency(0.02f);

    for (int y = 0; y < PIPELINE_HEIGHT; y++) {
        for (int x = 0; x < PIPELINE_WIDTH; x++) {
            int index = y * PIPELINE_WIDTH + x;

            float offsetX = (seed % 100000) * 1.0f;
            float offsetY = ((seed / 100000) % 100000) * 1.0f;

            float h = heightNoise.GetNoise((float)x + offsetX, (float)y + offsetY);
            
            // Normalize to 0-255
            uint8_t heightVal = (uint8_t)((h + 1.0f) * 127.5f);
            
            // Biome logic (Simplified)
            // 1: Deep Water, 2: Water, 3: Sand, 4: Grass, 5: Forest, 6: Mountain, 7: Snow
            uint8_t biome = 4; // Default Grass
            
            if (h < -0.3f) biome = 1; // Deep Water
            else if (h < 0.0f) biome = 2; // Water
            else if (h < 0.1f) biome = 3; // Sand
            else if (h < 0.4f) biome = 4; // Grass
            else if (h < 0.7f) biome = 5; // Forest
            else if (h < 0.9f) biome = 6; // Mountain
            else biome = 7; // Snow
            
            // Wealth noise
             float w = noise.GetNoise((float)x + 500, (float)y + 500);
             uint8_t wealth = (uint8_t)((w + 1.0f) * 127.5f);

            // Ruination noise
            float rVal = noise.GetNoise((float)x + 1000, (float)y + 1000); // Offset
            uint8_t ruination = (uint8_t)((rVal + 1.0f) * 127.5f);
            
            // Political regions (Voronoi-like or just cellular noise)
            float pVal = noise.GetNoise((float)x * 5.0f, (float)y * 5.0f); 
            uint8_t political = (uint8_t)((pVal + 1.0f) * 127.5f) % 8 + 1; // 1-8 IDs

            m_grid[index] = { biome, heightVal, political, wealth, ruination };
        }
    }
    // Debug Log: Check if grid is actually changing
    if (!m_grid.empty()) {
        int centerIdx = (PIPELINE_HEIGHT / 2) * PIPELINE_WIDTH + (PIPELINE_WIDTH / 2);
        auto logger = ServiceLocator::Get().GetService<ILoggerService>();
        if (logger) logger->Log("Center Tile Biome: " + std::to_string(m_grid[centerIdx].BiomeID) + 
                                ", Height: " + std::to_string(m_grid[centerIdx].Height));
    }
    // Generate Texture
    RegenerateTexture();
}

void MacroService::UpdateTexture()
{
    // Texture Dimensions: 256x256 grid * 8x8 font = 2048x2048 pixels
    constexpr int TEXTURE_WIDTH = PIPELINE_WIDTH * 8;
    constexpr int TEXTURE_HEIGHT = PIPELINE_HEIGHT * 8;
    
    std::vector<unsigned char> data(TEXTURE_WIDTH * TEXTURE_HEIGHT * 3, 0); // Initialize with black
    
    auto DrawGlyph = [&](int tileX, int tileY, uint8_t charCode, unsigned char r, unsigned char g, unsigned char b) {
        if (charCode >= 128) return;
        
        int startX = tileX * 8;
        int startY = tileY * 8;
        
        for (int row = 0; row < 8; ++row) {
            uint8_t rowBits = BITMAP_FONT[charCode][row];
            for (int col = 0; col < 8; ++col) {
                if (rowBits & (1 << (7 - col))) {
                    int px = startX + col;
                    int py = startY + row;
                    int idx = (py * TEXTURE_WIDTH + px) * 3;
                    if (idx < data.size()) {
                        data[idx + 0] = r;
                        data[idx + 1] = g;
                        data[idx + 2] = b;
                    }
                }
            }
        }
    };

    for (int i = 0; i < PIPELINE_WIDTH * PIPELINE_HEIGHT; ++i) {
        int x = i % PIPELINE_WIDTH;
        int y = i / PIPELINE_WIDTH;
        const auto& tile = m_grid[i];
        
        unsigned char r = 255, g = 255, b = 255;
        uint8_t glyph = '?';

        if (m_currentMode == MacroViewMode::Biome) {
            switch (tile.BiomeID) {
                case 1: r=0; g=0; b=200; glyph = '~'; break;   // Deep Water
                case 2: r=65; g=105; b=225; glyph = '~'; break; // Water
                case 3: r=238; g=214; b=175; glyph = '.'; break; // Sand
                case 4: r=34; g=139; b=34; glyph = ','; break;  // Grass
                case 5: r=0; g=100; b=0; glyph = 5; break;    // Forest (Tree club symbol)
                case 6: r=128; g=128; b=128; glyph = 94; break; // Mountain (^)
                case 7: r=240; g=248; b=255; glyph = '*'; break; // Snow
                default: r=255; g=0; b=255; glyph = '?'; break; 
            }
        }
        else if (m_currentMode == MacroViewMode::Political) {
             glyph = '#';
             switch (tile.PoliticalID % 6) {
                 case 0: r=255; g=0; b=0; break;
                 case 1: r=0; g=255; b=0; break;
                 case 2: r=100; g=100; b=255; break;
                 case 3: r=255; g=255; b=0; break;
                 case 4: r=0; g=255; b=255; break;
                 case 5: r=255; g=0; b=255; break;
             }
        }
        else if (m_currentMode == MacroViewMode::Height) {
            glyph = 127; // Full block
            r = g = b = tile.Height;
        }
        else {
            glyph = '?'; // Default
        }
        
        DrawGlyph(x, y, glyph, r, g, b);
    }

    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void MacroService::SetViewMode(MacroViewMode mode) {
    if (m_currentMode != mode) {
        m_currentMode = mode;
        UpdateTexture();
    }
}

unsigned int MacroService::GetMapTexture() {
    return m_textureID;
}

void MacroService::RegenerateTexture() {
    UpdateTexture();
}

