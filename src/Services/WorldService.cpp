#include "Services/WorldService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>

WorldService::WorldService()
{
}

void WorldService::Init()
{
    // Improved Noise Settings
    m_noise.SetNoiseType(FastNoiseLite::NoiseType::Perlin);
    m_noise.SetFractalType(FastNoiseLite::FractalType::FBm);
    m_noise.SetFrequency(0.01f); // Low frequency for large continents
    m_noise.SetFractalOctaves(4);
    m_noise.SetSeed(1337);

    ServiceLocator::Get().GetService<ILoggerService>()->Log("World Service Initialized (Terraced).");
}

void WorldService::Clean()
{
}

void WorldService::GenerateChunk(Chunk* chunk, int worldX, int worldY)
{
    // World coordinates (Chunk Coord * Size)
    int baseX = worldX * Chunk::SIZE;
    int baseZ = worldY * Chunk::SIZE;
    
    int baseHeight = 5; // Water Level
    int maxHeight = 20;

    for (int x = 0; x < Chunk::SIZE; x++)
    {
        for (int z = 0; z < Chunk::SIZE; z++)
        {
            // Global Coordinates
            float globalX = (float)(baseX + x);
            float globalZ = (float)(baseZ + z);

            // 1. Get Noise (-1 to 1)
            float rawNoise = m_noise.GetNoise(globalX, globalZ); 
            
            // 2. Normalize (0 to 1)
            float n = (rawNoise + 1.0f) * 0.5f;

            // 3. Terrace Logic
            int finalY = baseHeight;

            // Cut off for Water
            if (n < 0.3f) 
            {
                finalY = baseHeight; // Flat Water/Sand
            }
            else
            {
                // Quantize remaining range (0.3 to 1.0)
                float potential = (n - 0.3f) / 0.7f; // 0.0 to 1.0
                
                // Steps
                int addedHeight = (int)(potential * (maxHeight - baseHeight));
                finalY = baseHeight + addedHeight;
            }

            // Fill Column
            // We iterate up to finalY
            // Chunk Y is 0..15 usually.
            // With MaxHeight 20, we need to be careful if Chunk height is limited.
            // Chunk::SIZE is 16.
            // If finalY > 15, we clamp OR we need vertical chunks.
            // For now, let's Clamp to 15 to stay within bounds of a single chunk system 
            // OR if IsInBounds checks Y, we are fine?
            // Chunk::IsInBounds checks 0..SIZE (16).
            // So we MUST clamp to 15.
            
            int clampedY = finalY;
            if (clampedY >= Chunk::SIZE) clampedY = Chunk::SIZE - 1;

            for (int y = 0; y < Chunk::SIZE; y++)
            {
                if (y <= clampedY)
                {
                    uint8_t block = 0;

                     // Biome / Block Type
                    if (y <= baseHeight && n < 0.3f)
                    {
                         // Water Body
                         block = 5; // Water
                         // Optional: Sand at edges? (Complex without neighbor check)
                         // Simple: If close to 0.3?
                         if (n > 0.28f) block = 3; // Sand
                    }
                    else
                    {
                        // Land
                        if (y == clampedY) block = 2; // Grass on top
                        else block = 1; // Dirt below
                    }

                    chunk->SetBlock(x, y, z, block);
                }
                else
                {
                    chunk->SetBlock(x, y, z, 0); // Air
                }
            }
        }
    }
}
