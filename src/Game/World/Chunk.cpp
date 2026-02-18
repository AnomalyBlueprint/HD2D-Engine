#include "Game/World/Chunk.h"
#include "Engine/Services/World/IWorldService.h"
#include "Engine/Services/Rendering/TextureAtlasService.h"
#include "Engine/Services/World/IBlockRegistryService.h"
#include "Engine/Data/KenneyIDs.h"
#include "Engine/Core/Vertex.h"
#include <vector>
#include <array>
#include <cstring> 
#include <GL/glew.h>

// ... includes ...

Chunk::Chunk() {
    std::memset(m_blocks, 0, sizeof(m_blocks));
    m_meshID = 0;
    m_indexCount = 0;
}

Chunk::~Chunk() {
    if (m_meshID > 0) {
        glDeleteVertexArrays(1, &m_meshID);
    }
}

bool Chunk::IsInBounds(int x, int y, int z) {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE && z >= 0 && z < SIZE;
}

int Chunk::GetIndex(int x, int y, int z) const {
    return x + (y * SIZE) + (z * SIZE * SIZE);
}

void Chunk::SetBlock(int x, int y, int z, uint8_t blockID) {
    if (IsInBounds(x, y, z)) {
        m_blocks[GetIndex(x, y, z)] = blockID;
    }
}

uint8_t Chunk::GetBlock(int x, int y, int z) const {
    if (IsInBounds(x, y, z)) {
        return m_blocks[GetIndex(x, y, z)];
    }
    return 0; // Air
}

void Chunk::SetMesh(unsigned int id, unsigned int count) {
    if (m_meshID > 0) {
        glDeleteVertexArrays(1, &m_meshID);
    }
    m_meshID = id;
    m_indexCount = count;
}

void Chunk::RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                        TextureAtlasService* atlas, IWorldService* worldService, IBlockRegistryService* blockRegistry) {
    
    vertices.clear();
    indices.clear();
    int indexCount = 0;

    // Helper: Get Block Definition
    auto GetDef = [&](uint8_t id) -> const BlockDef& {
        if (blockRegistry) return blockRegistry->GetBlock(id);
        static BlockDef air = { KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, true, false };
        return air;
    };

    auto IsBlockTransparent = [&](uint8_t id) {
        if (id == 0) return true;
        return GetDef(id).isTransparent;
    };

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            for (int z = 0; z < SIZE; z++) {
                
                uint8_t currentBlock = GetBlock(x, y, z);
                if (currentBlock == 0) continue; 

                const BlockDef& def = GetDef(currentBlock);
                if (def.isTransparent) { /* handle transparent logic if needed */ }

                // Helper: Get Texture ID based on Face
                // 0=Front, 1=Back, 2=Right, 3=Left, 4=Top, 5=Bottom
                auto GetTextureForFace = [&](int face) -> KenneyIDs {
                     if (face == 4) return def.textureTop;
                     if (face == 5) return def.textureBottom;
                     return def.textureSide;
                };

                // Helper: Get UVs
                auto GetUVs = [&](KenneyIDs id) -> std::array<glm::vec2, 4> {
                     if (atlas) {
                        UVRect r = atlas->GetUVs(id);
                        return { glm::vec2(r.uMin, r.vMin), glm::vec2(r.uMax, r.vMin), glm::vec2(r.uMax, r.vMax), glm::vec2(r.uMin, r.vMax) };
                    }
                    return { glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1) };
                };

                float fX = (float)x * BLOCK_RENDER_SIZE;
                float fY = (float)y * BLOCK_RENDER_SIZE;
                float fZ = (float)z * BLOCK_RENDER_SIZE;
                float fS = BLOCK_RENDER_SIZE; 
                glm::vec4 color(1.0f); 

                // --- NEIGHBOR CHECKING ---

                // 1. FRONT (Z+) - Face 0
                bool drawFront = false;
                if (z < SIZE - 1) {
                    if (IsBlockTransparent(GetBlock(x, y, z + 1))) drawFront = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + x;
                    int globalZ = (m_chunkZ * SIZE) + (z + 1);
                    if (IsBlockTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawFront = true;
                } else { drawFront = true; }

                if (drawFront) {
                    auto uvs = GetUVs(GetTextureForFace(0));
                    glm::vec3 normal(0.0f, 0.0f, 1.0f);
                    vertices.push_back({ glm::vec3(fX,      fY,      fZ + fS), color, uvs[0], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ + fS), color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ + fS), color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 2. BACK (Z-) - Face 1
                bool drawBack = false;
                if (z > 0) {
                    if (IsBlockTransparent(GetBlock(x, y, z - 1))) drawBack = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + x;
                    int globalZ = (m_chunkZ * SIZE) + (z - 1);
                    if (IsBlockTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawBack = true;
                } else { drawBack = true; }

                if (drawBack) {
                    auto uvs = GetUVs(GetTextureForFace(1));
                    glm::vec3 normal(0.0f, 0.0f, -1.0f);
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ), color, uvs[0], 0.0f, normal }); 
                    vertices.push_back({ glm::vec3(fX,      fY,      fZ), color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ), color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ), color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 3. RIGHT (X+) - Face 2
                bool drawRight = false;
                if (x < SIZE - 1) {
                    if (IsBlockTransparent(GetBlock(x + 1, y, z))) drawRight = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + (x + 1);
                    int globalZ = (m_chunkZ * SIZE) + z;
                    if (IsBlockTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawRight = true;
                } else { drawRight = true; }

                if (drawRight) {
                     auto uvs = GetUVs(GetTextureForFace(2));
                     glm::vec3 normal(1.0f, 0.0f, 0.0f);
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ + fS), color, uvs[0], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ),      color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ),      color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 4. LEFT (X-) - Face 3
                bool drawLeft = false;
                if (x > 0) {
                    if (IsBlockTransparent(GetBlock(x - 1, y, z))) drawLeft = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + (x - 1);
                    int globalZ = (m_chunkZ * SIZE) + z;
                    if (IsBlockTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawLeft = true;
                } else { drawLeft = true; }

                if (drawLeft) {
                    auto uvs = GetUVs(GetTextureForFace(3));
                    glm::vec3 normal(-1.0f, 0.0f, 0.0f);
                    vertices.push_back({ glm::vec3(fX, fY,      fZ),      color, uvs[0], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX, fY,      fZ + fS), color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX, fY + fS, fZ + fS), color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX, fY + fS, fZ),      color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 5. TOP (Y+) - Face 4
                bool drawTop = false;
                if (y < SIZE - 1) {
                    if (IsBlockTransparent(GetBlock(x, y + 1, z))) drawTop = true;
                } else { drawTop = true; }

                if (drawTop) {
                    auto uvs = GetUVs(GetTextureForFace(4));
                    glm::vec3 normal(0.0f, 1.0f, 0.0f);
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ + fS), color, uvs[0], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ),      color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ),      color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 6. BOTTOM (Y-) - Face 5
                bool drawBottom = false;
                if (y > 0) {
                    if (IsBlockTransparent(GetBlock(x, y - 1, z))) drawBottom = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + x;
                    int globalZ = (m_chunkZ * SIZE) + z;
                    if (IsBlockTransparent(worldService->GetBlockAt(globalX, y - 1, globalZ))) drawBottom = true;
                } else { drawBottom = true; }

                if (drawBottom) {
                    auto uvs = GetUVs(GetTextureForFace(5));
                    glm::vec3 normal(0.0f, -1.0f, 0.0f);
                    vertices.push_back({ glm::vec3(fX,      fY, fZ),      color, uvs[0], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY, fZ),      color, uvs[1], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX + fS, fY, fZ + fS), color, uvs[2], 0.0f, normal });
                    vertices.push_back({ glm::vec3(fX,      fY, fZ + fS), color, uvs[3], 0.0f, normal });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }
            }
        }
    }
}