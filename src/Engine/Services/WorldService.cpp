#include "Engine/Services/WorldService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"
#include "Engine/Services/ChunkManager.h"
#include <iostream>

WorldService::WorldService()
{
}

void WorldService::OnInitialize()
{
    // Defer generation to GenerateInitialWorld
    ServiceLocator::Get().GetService<ILoggerService>()->Log("World Service Initialized (Waiting for Start).");
}

void WorldService::GenerateInitialWorld(int seed)
{
    // Improved Noise Settings
    m_noise.SetNoiseType(FastNoiseLite::NoiseType::Perlin);
    m_noise.SetFractalType(FastNoiseLite::FractalType::FBm);
    m_noise.SetFrequency(0.01f); // Low frequency for large continents
    m_noise.SetFractalOctaves(4);
    m_noise.SetSeed(seed);

    ServiceLocator::Get().GetService<ILoggerService>()->Log("World Generated with Seed: " + std::to_string(seed));
}

void WorldService::ClearWorld()
{
    auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();
    if (chunkManager)
    {
        chunkManager->Clear(); 
    }
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

            // Get Noise (-1 to 1) 
            float rawNoise = m_noise.GetNoise(globalX, globalZ); 
            
            // Normalize (0 to 1)
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
                         block = 5; // Water
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

uint8_t WorldService::GetBlockAt(int globalX, int globalY, int globalZ)
{
    // 1. Determine Chunk
    int chunkX = (int)floor((float)globalX / Chunk::SIZE);
    int chunkZ = (int)floor((float)globalZ / Chunk::SIZE);
    
    // 2. Query ChunkManager
    auto chunkManager = ServiceLocator::Get().GetService<ChunkManager>();
    if (chunkManager)
    {
        auto chunk = chunkManager->GetChunk(chunkX, chunkZ);
        if (chunk)
        {
            // Calculate Local Coords
            int localX = globalX % Chunk::SIZE;
            int localZ = globalZ % Chunk::SIZE;
            
            // Handle negative modulo correctly
            if (localX < 0) localX += Chunk::SIZE;
            if (localZ < 0) localZ += Chunk::SIZE;

            if (Chunk::IsInBounds(localX, globalY, localZ))
            {
                return chunk->GetBlock(localX, globalY, localZ);
            }
            return 0; // Air if out of vertical bounds of chunk
        }
    }

    // 3. Fallback: Generate Noise (if chunk not loaded)
    // We duplicate the generation logic for a single block column

    float rawNoise = m_noise.GetNoise((float)globalX, (float)globalZ); 
    float n = (rawNoise + 1.0f) * 0.5f;
    
    int baseHeight = 5;
    int maxHeight = 20;
    int finalY = baseHeight;

    if (n < 0.3f) 
    {
        finalY = baseHeight;
    }
    else
    {
        float potential = (n - 0.3f) / 0.7f;
        int addedHeight = (int)(potential * (maxHeight - baseHeight));
        finalY = baseHeight + addedHeight;
    }

    if (globalY <= finalY)
    {
        if (globalY <= baseHeight && n < 0.3f)
        {
             if (n > 0.28f) return 3; // Sand
             return 5; // Water
        }
        else
        {
            if (globalY == finalY) return 2; // Grass
            return 1; // Dirt
        }
    }

    return 0; // Air
}
