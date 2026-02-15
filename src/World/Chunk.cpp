#include "World/Chunk.h"
#include "Services/IWorldService.h"
#include "Services/TextureAtlasService.h"
#include "Data/KenneyIDs.h"
#include "Core/Vertex.h"
#include <vector>
#include <array>
#include <cstring> 
#include <GL/glew.h>

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

// Restored Helper
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

// Helper: 0 is Air
bool IsTransparent(uint8_t blockID) {
    return blockID == 0; 
}

void Chunk::RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, 
                        TextureAtlasService* atlas, IWorldService* worldService) {
    
    vertices.clear();
    indices.clear();
    int indexCount = 0;

    for (int x = 0; x < SIZE; x++) {
        for (int y = 0; y < SIZE; y++) {
            for (int z = 0; z < SIZE; z++) {
                
                uint8_t currentBlock = GetBlock(x, y, z);
                if (currentBlock == 0) continue; 

                KenneyIDs texID = static_cast<KenneyIDs>(currentBlock); 
                
                // --- FIX: Handle UVRect Conversion Manually ---
                std::array<glm::vec2, 4> uvs;
                if (atlas) {
                    UVRect r = atlas->GetUVs(texID); // Get struct first
                    uvs[0] = glm::vec2(r.uMin, r.vMin); // Bottom-Left
                    uvs[1] = glm::vec2(r.uMax, r.vMin); // Bottom-Right
                    uvs[2] = glm::vec2(r.uMax, r.vMax); // Top-Right
                    uvs[3] = glm::vec2(r.uMin, r.vMax); // Top-Left
                } else {
                    uvs = { glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1) };
                }

                float fX = (float)x * BLOCK_RENDER_SIZE;
                float fY = (float)y * BLOCK_RENDER_SIZE;
                float fZ = (float)z * BLOCK_RENDER_SIZE;
                float fS = BLOCK_RENDER_SIZE; 

                glm::vec4 color(1.0f); 

                // --- NEIGHBOR CHECKING ---

                // 1. FRONT (Z+)
                bool drawFront = false;
                if (z < SIZE - 1) {
                    if (IsTransparent(GetBlock(x, y, z + 1))) drawFront = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + x;
                    int globalZ = (m_chunkZ * SIZE) + (z + 1);
                    if (IsTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawFront = true;
                } else { drawFront = true; }

                if (drawFront) {
                    vertices.push_back({ glm::vec3(fX,      fY,      fZ + fS), color, uvs[0], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ + fS), color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ + fS), color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 2. BACK (Z-)
                bool drawBack = false;
                if (z > 0) {
                    if (IsTransparent(GetBlock(x, y, z - 1))) drawBack = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + x;
                    int globalZ = (m_chunkZ * SIZE) + (z - 1);
                    if (IsTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawBack = true;
                } else { drawBack = true; }

                if (drawBack) {
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ), color, uvs[0], 0.0f }); 
                    vertices.push_back({ glm::vec3(fX,      fY,      fZ), color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ), color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ), color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 3. RIGHT (X+)
                bool drawRight = false;
                if (x < SIZE - 1) {
                    if (IsTransparent(GetBlock(x + 1, y, z))) drawRight = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + (x + 1);
                    int globalZ = (m_chunkZ * SIZE) + z;
                    if (IsTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawRight = true;
                } else { drawRight = true; }

                if (drawRight) {
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ + fS), color, uvs[0], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY,      fZ),      color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ),      color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 4. LEFT (X-)
                bool drawLeft = false;
                if (x > 0) {
                    if (IsTransparent(GetBlock(x - 1, y, z))) drawLeft = true;
                } else if (worldService) {
                    int globalX = (m_chunkX * SIZE) + (x - 1);
                    int globalZ = (m_chunkZ * SIZE) + z;
                    if (IsTransparent(worldService->GetBlockAt(globalX, y, globalZ))) drawLeft = true;
                } else { drawLeft = true; }

                if (drawLeft) {
                    vertices.push_back({ glm::vec3(fX, fY,      fZ),      color, uvs[0], 0.0f });
                    vertices.push_back({ glm::vec3(fX, fY,      fZ + fS), color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX, fY + fS, fZ + fS), color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX, fY + fS, fZ),      color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 5. TOP (Y+)
                bool drawTop = false;
                if (y < SIZE - 1) {
                    if (IsTransparent(GetBlock(x, y + 1, z))) drawTop = true;
                } else { drawTop = true; }

                if (drawTop) {
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ + fS), color, uvs[0], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ + fS), color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY + fS, fZ),      color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX,      fY + fS, fZ),      color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }

                // 6. BOTTOM (Y-)
                bool drawBottom = false;
                if (y > 0) {
                    if (IsTransparent(GetBlock(x, y - 1, z))) drawBottom = true;
                } 

                if (drawBottom) {
                    vertices.push_back({ glm::vec3(fX,      fY, fZ),      color, uvs[0], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY, fZ),      color, uvs[1], 0.0f });
                    vertices.push_back({ glm::vec3(fX + fS, fY, fZ + fS), color, uvs[2], 0.0f });
                    vertices.push_back({ glm::vec3(fX,      fY, fZ + fS), color, uvs[3], 0.0f });
                    indices.push_back(indexCount + 0); indices.push_back(indexCount + 1); indices.push_back(indexCount + 2);
                    indices.push_back(indexCount + 2); indices.push_back(indexCount + 3); indices.push_back(indexCount + 0);
                    indexCount += 4;
                }
            }
        }
    }
}