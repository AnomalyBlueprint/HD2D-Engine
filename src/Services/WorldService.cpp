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
    m_noise.SetFrequency(0.02f);
    m_noise.SetFractalOctaves(4);
    m_noise.SetSeed(1337);

    ServiceLocator::Get().GetService<ILoggerService>()->Log("World Service Initialized (Infinite).");
}

void WorldService::Clean()
{
}

void WorldService::GenerateChunk(Chunk* chunk, int worldX, int worldY)
{
    // World coordinates (Chunk Coord * Size)
    // Note: worldX, worldY are actually Chunk Coordinates passed from ChunkManager
    
    for (int x = 0; x < Chunk::SIZE; x++)
    {
        for (int z = 0; z < Chunk::SIZE; z++)
        {
            // Global Coordinates for Noise
            float globalX = (float)(worldX * Chunk::SIZE + x);
            float globalZ = (float)(worldY * Chunk::SIZE + z);

            // Generated Height (0..1)
            float n = m_noise.GetNoise(globalX, globalZ); 
            
            // Normalize (-1..1) to (0..1)
            // But Perlin usually mostly -1..1.
            // Let's create hills.
            // Scale height to 1..30 blocks?
            // Base height + Noise * Amp
            
            float heightMap = (n + 1.0f) * 0.5f; // 0..1
            int height = (int)(heightMap * 20.0f) + 4; // 4 to 24 height
            
            for (int y = 0; y < Chunk::SIZE * 2; y++) // Allow higher chunks? 
            {
                // Chunk::SIZE is 16. If height > 16, we clamp?
                // Our Chunk only supports 0..15 Y currently (IsInBounds).
                // "Simple Perlin terrain" says "Chunk::SIZE".
                // We should probably clamp height to 15 for now until we support Vertical Chunks.
                if (y >= Chunk::SIZE) break;

                if (y <= height)
                {
                    // Block Selection (Biomes via Height)
                    uint8_t blockType = 1; // Dirt

                    if (y == height)
                    {
                        // Top Layer
                        if (y < 5) blockType = 17; // Sand? Need KenneyIDs check.
                        // Assuming 17 is Sand in my logic or using explicit mapping?
                        // Chunk.cpp re-maps IDs to KenneyIDs.
                        // We should establish a convention.
                        // Let's stick to:
                        // 1 = Dirt
                        // 2 = Grass
                        // 3 = Sand
                        // 4 = Snow
                        // 5 = Water
                        
                        if (y < 6) blockType = 3; // Sand
                        else if (y > 12) blockType = 4; // Snow
                        else blockType = 2; // Grass
                    }
                    else
                    {
                         // Below Surface
                         blockType = 1; // Dirt
                         if (y > 12) blockType = 1; // Dirt under Snow is still Dirt? Or Stone?
                         // Let's use Stone for deep?
                         if (y < height - 3) blockType = 6; // Stone (ID 6)
                    }

                    chunk->SetBlock(x, y, z, blockType);
                }
                else if (y < 5)
                {
                    // Water Level
                    chunk->SetBlock(x, y, z, 5); // Water
                }
                else
                {
                    chunk->SetBlock(x, y, z, 0); // Air
                }
            }
        }
    }
}
