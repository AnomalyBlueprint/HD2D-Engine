#include "Services/ChunkManager.h"
#include "Services/ServiceLocator.h"
#include "Services/IWorldService.h"
#include "Services/TextureAtlasService.h"
#include "Services/ILoggerService.h"
#include "Services/RenderService.h" 
#include "World/Chunk.h"
#include "Core/Vertex.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include "Core/GameConfig.h"

// const float CHUNK_PIXEL_SIZE = 32.0f * 32.0f; // Moved to GameConfig

ChunkManager::ChunkManager() {}

ChunkManager::~ChunkManager() 
{
    Clean();
}

void ChunkManager::Init()
{
    auto logger = ServiceLocator::Get().GetService<ILoggerService>();
    if(logger) logger->Log("Chunk Manager Initialized.");
}

void ChunkManager::Clean()
{
    m_activeChunks.clear();
}

// REMOVED: GetChunkKey implementation because it is already defined in the header file.

void ChunkManager::Update(glm::vec3 focusPoint)
{
    // Calculate Center Chunk from World Focus Point
    int chunkX = (int)floor(focusPoint.x / GameConfig::CHUNK_PIXEL_SIZE);
    int chunkZ = (int)floor(focusPoint.z / GameConfig::CHUNK_PIXEL_SIZE);

    int currentX = chunkX;
    int currentZ = chunkZ;

    auto worldService = ServiceLocator::Get().GetService<IWorldService>();
    auto atlasService = ServiceLocator::Get().GetService<TextureAtlasService>();

    // FIX: Use the class member m_renderDistance (fixes unused variable warning)
    int radius = GameConfig::RENDER_RADIUS; 

    for (int x = -radius; x <= radius; x++)
    {
        for (int z = -radius; z <= radius; z++)
        {
            int targetX = currentX + x;
            int targetZ = currentZ + z;
            
            auto key = GetChunkKey(targetX, targetZ);
            
            if (m_activeChunks.find(key) == m_activeChunks.end())
            {
                auto chunk = std::make_shared<Chunk>();
                // CRITICAL: Set ID so Chunk knows its global position for seam stitching
                chunk->SetCoordinates(targetX, targetZ);
                
                // Generate
                if (worldService) {
                    worldService->GenerateChunk(chunk.get(), targetX, targetZ);
                }
                
                // Meshing
                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                
                // PASS SERVICES: worldService allows Chunk to peek at neighbors
                chunk->RebuildMesh(vertices, indices, atlasService.get(), worldService.get());
                
                // Upload
                if (!vertices.empty())
                {
                    std::vector<float> floatVertices;
                    floatVertices.reserve(vertices.size() * 10);
                    
                    for(const auto& v : vertices)
                    {
                        floatVertices.push_back(v.position.x); floatVertices.push_back(v.position.y); floatVertices.push_back(v.position.z);
                        floatVertices.push_back(v.color.r); floatVertices.push_back(v.color.g); floatVertices.push_back(v.color.b); floatVertices.push_back(v.color.a);
                        floatVertices.push_back(v.texCoord.x); floatVertices.push_back(v.texCoord.y);
                        floatVertices.push_back(v.textureID);
                    }
                    
                    auto renderer = ServiceLocator::Get().GetService<RenderService>();
                    if (renderer)
                    {
                        unsigned int meshID = renderer->CreateMesh(floatVertices, indices);
                        chunk->SetMesh(meshID, indices.size());
                    }
                }
                 
                m_activeChunks[key] = chunk;
            }
        }
    }
    
    // Unload far chunks
    for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); )
    {
        int cx = it->first.first;
        int cz = it->first.second;
        
        if (std::abs(cx - currentX) > radius + 1 || std::abs(cz - currentZ) > radius + 1)
        {
            it = m_activeChunks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::shared_ptr<Chunk> ChunkManager::GetChunk(int x, int y)
{
    auto key = GetChunkKey(x, y);
    if (m_activeChunks.find(key) != m_activeChunks.end())
    {
        return m_activeChunks[key];
    }
    return nullptr;
}

void ChunkManager::Render(RenderService* renderer, IShaderService* shader)
{
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    int modelLoc = glGetUniformLocation(currentProgram, "model");

    for (const auto& pair : m_activeChunks)
    {
        auto key = pair.first;
        auto chunk = pair.second;
        
        if (chunk->GetMeshID() == 0) continue;
        
        // Translate Mesh
        float posX = key.first * GameConfig::CHUNK_PIXEL_SIZE;
        float posZ = key.second * GameConfig::CHUNK_PIXEL_SIZE;
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(posX, 0.0f, posZ));
        
        if (modelLoc >= 0)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        }
        
        renderer->DrawMesh(chunk->GetMeshID(), chunk->GetIndexCount());
    }
}