#pragma once
#include "Services/IService.h"
#include "Services/RenderService.h"
#include "Services/IShaderService.h"
#include "World/Chunk.h"
#include <map>
#include <memory>
#include <glm/glm.hpp>

class ChunkManager : public IService
{
public:
    ChunkManager();
    ~ChunkManager();

    void Init() override;
    void Update() override {} // Unused, we use Update(vec2)
    void Clean() override;

    void Update(glm::vec3 focusPoint);

    void Render(RenderService* renderer, IShaderService* shader);

    // Helper for WorldService to query active chunks
    std::shared_ptr<Chunk> GetChunk(int x, int y);

private:
    std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_activeChunks;
    
    int m_renderDistance = 2;
    const float CHUNK_PIXEL_SIZE = Chunk::SIZE * Chunk::BLOCK_RENDER_SIZE;

    // Helper to generate key
    std::pair<int, int> GetChunkKey(int x, int y) { return {x, y}; }
};
