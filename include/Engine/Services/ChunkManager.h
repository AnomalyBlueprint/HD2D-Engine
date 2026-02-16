#pragma once
#include "Engine/Services/IService.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IShaderService.h"
#include "Engine/Services/IBlockRegistryService.h"
#include "Game/World/Chunk.h"
#include <map>
#include <memory>
#include <glm/glm.hpp>

class ChunkManager : public IService
{
public:
    ChunkManager();
    ~ChunkManager();

    /// <summary>
    /// Initializes the ChunkManager service.
    /// </summary>
    void Init() override;

    /// <summary>
    /// Unused update method from IService. We use the overload with position instead.
    /// </summary>
    void Update() override {} 

    /// <summary>
    /// Cleans up resources used by the ChunkManager.
    /// </summary>
    void Clean() override;

    /// <summary>
    /// Manages the active voxel chunks, handling loading, meshing, and unloading based on player position.
    /// </summary>
    /// <param name="focusPoint">The world-space coordinate to center the loading radius on.</param>
    void Update(glm::vec3 focusPoint);

    /// <summary>
    /// Renders all active chunks using the provided renderer and shader.
    /// </summary>
    /// <param name="renderer">The render service to use for drawing.</param>
    /// <param name="shader">The shader service to use for setting uniforms.</param>
    void Render(RenderService* renderer, IShaderService* shader);

    /// <summary>
    /// Retrieves a pointer to the chunk as the specified grid coordinates.
    /// </summary>
    /// <param name="x">The X grid coordinate of the chunk.</param>
    /// <param name="y">The Y (Z) grid coordinate of the chunk.</param>
    /// <returns>A shared pointer to the chunk if found, otherwise nullptr.</returns>
    std::shared_ptr<Chunk> GetChunk(int x, int y);

private:
    std::map<std::pair<int, int>, std::shared_ptr<Chunk>> m_activeChunks;
    
    /// <summary>
    /// Helper to generate a unique key for the chunk map.
    /// </summary>
    /// <param name="x">The X grid coordinate.</param>
    /// <param name="y">The Y (Z) grid coordinate.</param>
    /// <returns>A pair representing the key.</returns>
    std::pair<int, int> GetChunkKey(int x, int y);
};
