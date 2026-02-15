#include "World/Chunk.h"
#include <cstring>

Chunk::Chunk()
{
    // Initialize as Air (0)
    std::memset(m_blocks, 0, sizeof(m_blocks));
}

bool Chunk::IsInBounds(int x, int y, int z)
{
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE && z >= 0 && z < SIZE;
}

int Chunk::GetIndex(int x, int y, int z)
{
    return x + y * SIZE + z * SIZE * SIZE;
}

void Chunk::SetBlock(int x, int y, int z, uint8_t id)
{
    if (IsInBounds(x, y, z))
    {
        m_blocks[GetIndex(x, y, z)] = id;
    }
}

uint8_t Chunk::GetBlock(int x, int y, int z) const
{
    if (IsInBounds(x, y, z))
    {
        return m_blocks[GetIndex(x, y, z)];
    }
    return 0; // Return 0 (Air) if out of bounds
}

void Chunk::RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();

    for (int x = 0; x < SIZE; x++)
    {
        for (int y = 0; y < SIZE; y++)
        {
            for (int z = 0; z < SIZE; z++)
            {
                uint8_t block = GetBlock(x, y, z);
                if (block == 0) continue; // Skip Air

                // Color Selection
                glm::vec4 color(1.0f);
                if (block == 1) // Dirt
                    color = glm::vec4(0.6f, 0.3f, 0.0f, 1.0f);
                else if (block == 2) // Grass
                    color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);

                // Check neighbors for culling
                
                // TOP (Y+1)
                if (GetBlock(x, y + 1, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 0, color);
                }
                // BOTTOM (Y-1)
                if (GetBlock(x, y - 1, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 1, color);
                }
                // LEFT (X-1)
                if (GetBlock(x - 1, y, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 2, color);
                }
                // RIGHT (X+1)
                if (GetBlock(x + 1, y, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 3, color);
                }
                // FRONT (Z+1)
                if (GetBlock(x, y, z + 1) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 4, color);
                }
                // BACK (Z-1)
                if (GetBlock(x, y, z - 1) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 5, color);
                }
            }
        }
    }
}

void Chunk::AddFace(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, int face, const glm::vec4& baseColor)
{
    // Face IDs: 0=Top, 1=Bottom, 2=Left, 3=Right, 4=Front, 5=Back
    float fx = (float)x * BLOCK_RENDER_SIZE;
    float fy = (float)y * BLOCK_RENDER_SIZE;
    float fz = (float)z * BLOCK_RENDER_SIZE;
    float fs = BLOCK_RENDER_SIZE;

    // Apply some shading
    float shade = 1.0f;
    if (face == 1) shade = 0.5f; // Bottom
    else if (face == 2 || face == 3) shade = 0.8f; // Sides
    else if (face == 4 || face == 5) shade = 0.9f; // Front/Back

    glm::vec4 color = baseColor * glm::vec4(shade, shade, shade, 1.0f);
    color.a = 1.0f;

    // TextureID = 1.0f (No Texture / White Texture fallback behavior if we use 1.0 as "None")
    // Wait, 0 was White Texture (Fallback).
    // The prompt says: "Use TextureID = 1.0f (Use "No Texture" mode for now...)"
    // But earlier we implemented 0 as White Texture Fallback in UseTexture(0).
    // If I pass 1.0f to the shader attribute `aTexID`:
    // The shader effectively ignores `aTexID` unless we use it for batching logic in Fragment Shader.
    // Currently `basic.frag` does `texture(ourTexture, TexCoord) * VertexColor`.
    // It uses whatever texture is bound.
    // If we bind White Texture (ID=0 -> mapped to m_whiteTextureID), then `texture(...)` returns White.
    // So `VertexColor` becomes the visible color.
    // So `UseTexture(0)` in Game.cpp is correct.
    // The `textureID` in Vertex struct is for batching if we use a texture array or logic to select texture.
    // For now, let's set it to 0.0f to match the fallback? 
    // Or 1.0f if the user insisted? 
    // The prompt says: "Texture ID: Set Vertex.textureID = 1.0f (Use "No Texture" mode for now, relying on Vertex Colors)."
    // I will set it to 1.0f but in Game.cpp we must ensure we bind White Texture if using this mode, OR we update shaders to use `aTexID` to mix.
    // But since we are NOT batching multiple textures yet (just one draw call per chunk), we just need to bind White Texture.
    // `aTexID` is currently unused in fragment shader logic (it's just passed or ignored). 
    // I will set it to 1.0f as requested.
    
    float texID = 1.0f; 

    unsigned int startIndex = vertices.size();

    auto PushVert = [&](float vx, float vy, float vz, float u, float v)
    {
        Vertex vert;
        vert.position = glm::vec3(vx, vy, vz);
        vert.color = color;
        vert.texCoord = glm::vec2(u, v);
        vert.textureID = texID;
        vertices.push_back(vert);
    };

    // Offsets
    // Top: Y+1
    if(face == 0) // Top
    {
        PushVert(fx, fy+fs, fz,   0, 0);
        PushVert(fx+fs, fy+fs, fz, 1, 0);
        PushVert(fx+fs, fy+fs, fz+fs, 1, 1);
        PushVert(fx, fy+fs, fz+fs, 0, 1);
    }
    // Bottom: Y
    else if(face == 1) // Bottom
    {
        PushVert(fx, fy, fz+fs, 0, 0);
        PushVert(fx+fs, fy, fz+fs, 1, 0);
        PushVert(fx+fs, fy, fz, 1, 1);
        PushVert(fx, fy, fz, 0, 1);
    }
    // Left: X
    else if(face == 2)
    {
        PushVert(fx, fy+fs, fz+fs, 1, 0);
        PushVert(fx, fy+fs, fz, 0, 0);
        PushVert(fx, fy, fz, 0, 1);
        PushVert(fx, fy, fz+fs, 1, 1);
    }
    // Right: X+1
    else if(face == 3)
    {
        PushVert(fx+fs, fy+fs, fz, 1, 0);
        PushVert(fx+fs, fy+fs, fz+fs, 0, 0);
        PushVert(fx+fs, fy, fz+fs, 0, 1);
        PushVert(fx+fs, fy, fz, 1, 1);
    }
    // Front: Z+1
    else if(face == 4)
    {
        PushVert(fx+fs, fy+fs, fz+fs, 1, 0);
        PushVert(fx, fy+fs, fz+fs, 0, 0);
        PushVert(fx, fy, fz+fs, 0, 1);
        PushVert(fx+fs, fy, fz+fs, 1, 1);
    }
    // Back: Z
    else if(face == 5)
    {
        PushVert(fx, fy+fs, fz, 1, 0);
        PushVert(fx+fs, fy+fs, fz, 0, 0);
        PushVert(fx+fs, fy, fz, 0, 1);
        PushVert(fx, fy, fz, 1, 1);
    }

    // Indices (0, 1, 2, 2, 3, 0)
    indices.push_back(startIndex + 0);
    indices.push_back(startIndex + 1);
    indices.push_back(startIndex + 2);

    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 3);
    indices.push_back(startIndex + 0);
}
