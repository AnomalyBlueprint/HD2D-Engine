#pragma once
#include <vector>
#include <cstdint>
#include "Core/Vertex.h"

// Forward Declarations
class TextureAtlasService;
class IWorldService;

class Chunk {
public:
    static const int SIZE = 32; 
    static constexpr float BLOCK_RENDER_SIZE = 32.0f; // Restored for ChunkManager

    Chunk();
    ~Chunk();

    // Helper: Validates coordinates (Restored for WorldService)
    static bool IsInBounds(int x, int y, int z);
    int GetIndex(int x, int y, int z) const;

    void SetBlock(int x, int y, int z, uint8_t blockID);
    uint8_t GetBlock(int x, int y, int z) const;

    // Coordinate System
    void SetCoordinates(int x, int z) { m_chunkX = x; m_chunkZ = z; }
    int GetX() const { return m_chunkX; }
    int GetZ() const { return m_chunkZ; }

    // Mesh Generation
    void RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                     TextureAtlasService* atlas, IWorldService* worldService);

    // Render Data
    void SetMesh(unsigned int id, unsigned int count);
    unsigned int GetMeshID() const { return m_meshID; }
    unsigned int GetIndexCount() const { return m_indexCount; }

private:
    uint8_t m_blocks[SIZE * SIZE * SIZE];
    
    int m_chunkX = 0;
    int m_chunkZ = 0;

    unsigned int m_meshID = 0;
    unsigned int m_indexCount = 0;
};