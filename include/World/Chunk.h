#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <glm/glm.hpp>
#include "Core/Vertex.h"

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

    // Rebuilds mesh and populates vertices/indices vectors
    void RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);

private:
    uint8_t m_blocks[SIZE * SIZE * SIZE];

    void AddFace(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, int face, const glm::vec4& color);
};
