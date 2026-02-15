#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>
#include "Core/Vertex.h"
#include "Services/TextureAtlasService.h"

class Chunk
{
public:
    static const int SIZE = 16;
    static constexpr float BLOCK_RENDER_SIZE = 32.0f;

    Chunk();

    // Helper: x, y, z must be 0..15
    static bool IsInBounds(int x, int y, int z);
    static int GetIndex(int x, int y, int z);

    void SetBlock(int x, int y, int z, uint8_t id);
    uint8_t GetBlock(int x, int y, int z) const;

    // Coordinates (Needed for Seamless Meshing)
    void SetCoordinates(int x, int z) { m_chunkX = x; m_chunkZ = z; }
    int GetChunkX() const { return m_chunkX; }
    int GetChunkZ() const { return m_chunkZ; }

    // Rebuilds mesh and populates vertices/indices vectors
    // Needs WorldService for neighbor checking
    void RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, TextureAtlasService* atlas, class IWorldService* worldService);

    // Mesh Data
    unsigned int GetMeshID() const { return m_meshID; }
    int GetIndexCount() const { return m_indexCount; }
    void SetMesh(unsigned int id, int count);
    
    // Destructor to clean up GPU buffers
    ~Chunk();

private:
    uint8_t m_blocks[SIZE * SIZE * SIZE];
    
    unsigned int m_meshID = 0;
    int m_indexCount = 0;
    
    int m_chunkX = 0;
    int m_chunkZ = 0;

    void AddFace(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, int face, const glm::vec4& color, UVRect uv);
};
