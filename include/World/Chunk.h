#pragma once
#include <vector>
#include <cstdint>
#include "Core/Vertex.h"

// Forward Declarations
class TextureAtlasService;
class IWorldService;
class IBlockRegistryService;

/// <summary>
/// Represents a cubic volume of voxel data (32x32x32).
/// Handles block storage and mesh generation.
/// </summary>
class Chunk {
public:
    static const int SIZE = 32;                     ///< Dimensions of the chunk (X, Y, Z).
    static constexpr float BLOCK_RENDER_SIZE = 32.0f; ///< World-space size of a single block.

    Chunk();
    ~Chunk();

    /// <summary>
    /// Checks if the given local coordinates are within the chunk bounds.
    /// </summary>
    static bool IsInBounds(int x, int y, int z);

    /// <summary>
    /// Calculates the 1D array index for the given 3D coordinates.
    /// </summary>
    int GetIndex(int x, int y, int z) const;

    /// <summary>
    /// Sets the block ID at the specified local coordinates.
    /// </summary>
    void SetBlock(int x, int y, int z, uint8_t blockID);

    /// <summary>
    /// Gets the block ID at the specified local coordinates.
    /// </summary>
    uint8_t GetBlock(int x, int y, int z) const;

    /// <summary>
    /// Sets the grid coordinates of this chunk in the world.
    /// </summary>
    void SetCoordinates(int x, int z) { m_chunkX = x; m_chunkZ = z; }
    int GetX() const { return m_chunkX; }
    int GetZ() const { return m_chunkZ; }

    /// <summary>
    /// Generates the mesh data for this chunk based on its block data and neighbors.
    /// </summary>
    /// <param name="vertices">Output vector for vertices.</param>
    /// <param name="indices">Output vector for indices.</param>
    /// <param name="atlas">Texture atlas service for UV mapping.</param>
    /// <param name="worldService">World service for neighbor lookup (seamlessnes).</param>
    /// <param name="blockRegistry">Registry service for block texture lookups.</param>
    void RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                     TextureAtlasService* atlas, IWorldService* worldService, IBlockRegistryService* blockRegistry);

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