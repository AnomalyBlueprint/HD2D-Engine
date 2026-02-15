#include "Services/ChunkManager.h"
#include "Services/ServiceLocator.h"
#include "Services/IWorldService.h"
#include "Services/TextureAtlasService.h"
#include "Services/ILoggerService.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h> 

ChunkManager::ChunkManager() {}

ChunkManager::~ChunkManager() 
{
    Clean();
}

void ChunkManager::Init()
{
    ServiceLocator::Get().GetService<ILoggerService>()->Log("Chunk Manager Initialized.");
}

void ChunkManager::Clean()
{
    m_activeChunks.clear();
}

void ChunkManager::Update(glm::vec3 focusPoint)
{
    // Calculate Center Chunk from World Focus Point (X, 0, Z)
    // ChunkSize = 16 * 32 = 512.
    int chunkX = (int)floor(focusPoint.x / CHUNK_PIXEL_SIZE);
    int chunkZ = (int)floor(focusPoint.z / CHUNK_PIXEL_SIZE);

    int currentX = chunkX;
    int currentZ = chunkZ;

    auto worldService = ServiceLocator::Get().GetService<IWorldService>();
    auto atlasService = ServiceLocator::Get().GetService<TextureAtlasService>();

    // Load new chunks in 5x5 Grid (Radius 2)
    int radius = 2; // As requested: -2 to +2
    for (int x = -radius; x <= radius; x++)
    {
        for (int z = -radius; z <= radius; z++)
        {
            int targetX = currentX + x;
            int targetZ = currentZ + z;
            
            auto key = GetChunkKey(targetX, targetZ);
            
            if (m_activeChunks.find(key) == m_activeChunks.end())
            {
                // Instantiate
                auto chunk = std::make_shared<Chunk>();
                chunk->SetCoordinates(targetX, targetZ);
                
                // Generate Data
                worldService->GenerateChunk(chunk.get(), targetX, targetZ);
                
                // Mesh
                std::vector<Vertex> vertices;
                std::vector<unsigned int> indices;
                
                // Pass WorldService to RebuildMesh for Neighbor Checks
                chunk->RebuildMesh(vertices, indices, atlasService.get(), worldService.get());
                
                // Upload
                if (!vertices.empty())
                {
                     // Convert to float for CreateMesh
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
    
    // Cleanup old chunks (Distance > Radius + 1 buffer)
    for (auto it = m_activeChunks.begin(); it != m_activeChunks.end(); )
    {
        int cx = it->first.first;
        int cz = it->first.second;
        
        if (abs(cx - currentX) > radius + 1 || abs(cz - currentZ) > radius + 1)
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
    // Need to set Texture Atlas?
    // Game.cpp does `renderer->UseTexture(atlasID)`.
    // It's efficient to do it once before calling this Render, or here.
    // Let's assume Game.cpp sets the texture state.

    // Shader Uniforms:
    // We need to set MODEL MATRIX for each chunk.
    // "basic.vert" has `uniform mat4 model;` ? 
    // I previously checked and it DOES NOT have model matrix in the provided file content?
    // Wait, let's re-read basic.vert.
    // "gl_Position = projection * view * vec4(aPos, 1.0);"
    // It assumes aPos is World Space.
    // But our Chunks are generated in Local Coordinates (0..16).
    // If we want multiple chunks, we MUST translate them.
    // EITHER:
    // 1. Generate vertices in World Space (in GenerateChunk/RebuildMesh).
    // 2. Use Model Matrix in Shader.
    
    // Plan Task 1 says: "Crucial: You must apply a Model Matrix translation... glm::translate(...)"
    // This implies we SHOULD use Model Matrix.
    // BUT basic.vert I viewed earlier (in Step 526) showed:
    // "gl_Position = projection * view * vec4(aPos, 1.0);"
    // It was MISSING `model`.
    
    // I MUST UPDATE basic.vert TO SUPPORT MODEL MATRIX.
    // I will add that to the Execution steps.
    
    // For now, I will write the code to set the uniform assuming the shader supports it.
    
    // unsigned int shaderID = 0; // We define it or get it? 
    // IShaderService interface might not expose "GetProgramID"?
    // In Game.cpp `basicShaderID` is stored.
    // This `Render` method takes `IShaderService*`, but maybe we need the Program ID directly
    // or `IShaderService` has `SetMat4`?
    // Let's check `OpenGLShaderService`.
    
    // Assuming we can set uniforms via GL directly for now using `glUniformMatrix4fv`.
    // We need the program ID used.
    // `renderer` doesn't expose it.
    // We might need to pass `GLuint shaderProgramID` to Render.
    // Or simpler: Just rely on ServiceLocator to get ShaderService cast to OpenGLShaderService which implies we know the implementation.
    
    // Let's look at `IShaderService.h`? 
    // Or just use `glGetIntegerv(GL_CURRENT_PROGRAM, &id);` inside the loop (slow but works).
    
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    
    int modelLoc = glGetUniformLocation(currentProgram, "model");

    for (const auto& pair : m_activeChunks)
    {
        auto key = pair.first;
        auto chunk = pair.second;
        
        if (chunk->GetMeshID() == 0) continue;
        
        // Calculate Translation
        float posX = key.first * CHUNK_PIXEL_SIZE;
        float posZ = key.second * CHUNK_PIXEL_SIZE; // Y in 2D map -> Z in 3D
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(posX, 0.0f, posZ));
        
        if (modelLoc >= 0)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        }
        
        renderer->DrawMesh(chunk->GetMeshID(), chunk->GetIndexCount());
    }
}
