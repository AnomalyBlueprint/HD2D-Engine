#include "Services/WorldService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>

WorldService::WorldService()
{
}

void WorldService::Init()
{
    m_noise.SetNoiseType(FastNoiseLite::NoiseType::OpenSimplex2);
    m_noise.SetSeed(1337);
    ServiceLocator::Get().GetService<ILoggerService>()->Log("World Service Initialized.");
}

void WorldService::Clean()
{
}

void WorldService::GenerateChunk(Chunk* chunk, int worldX, int worldY)
{
    // Simple Perlin terrain
    for (int x = 0; x < Chunk::SIZE; x++)
    {
        for (int z = 0; z < Chunk::SIZE; z++)
        {
            // Global Coordinates
            float globalX = (float)(worldX * Chunk::SIZE + x);
            float globalZ = (float)(worldY * Chunk::SIZE + z);

            // Height Map (0 to 16)
            // Noise returns -1 to 1.
            float n = m_noise.GetNoise(globalX, globalZ); // -1..1
            
            // Normalize to 0..1
            // n = (n + 1.0f) * 0.5f; 

            // Scale to height (1..15)
            // Let's make it 4..12 to be safe
            int height = (int)((n + 1.0f) * 0.5f * (Chunk::SIZE - 2)) + 1;
            
            for (int y = 0; y < Chunk::SIZE; y++)
            {
                if (y < height)
                {
                    chunk->SetBlock(x, y, z, 1); // Dirt
                }
                else if (y == height)
                {
                    chunk->SetBlock(x, y, z, 2); // Grass
                }
                else
                {
                    chunk->SetBlock(x, y, z, 0); // Air
                }
            }
        }
    }
}
