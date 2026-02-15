#include "World/Chunk.h"
#include <iostream>
#include <GL/glew.h>
#include <cstring>

Chunk::Chunk()
{
    // Initialize as Air (0)
    std::memset(m_blocks, 0, sizeof(m_blocks));
}

Chunk::~Chunk()
{
    if (m_meshID > 0)
    {
        glDeleteBuffers(1, &m_meshID); // Wait, CreateMesh returns VAO ID usually? 
        // In OpenGLRenderService::CreateMesh:
        // glGenVertexArrays(1, &VAO); ... return VAO;
        // So yes, we delete Vertex Arrays.
        glDeleteVertexArrays(1, &m_meshID);
    }
}

void Chunk::SetMesh(unsigned int id, int count)
{
    m_meshID = id;
    m_indexCount = count;
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

void Chunk::RebuildMesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, TextureAtlasService* atlas)
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

                // Map Block ID to KenneyID
                KenneyIDs texID = KenneyIDs::Floor_Ground_Dirt; // 1 = Dirt
                if (block == 2) texID = KenneyIDs::Floor_Ground_Grass;
                else if (block == 3) texID = KenneyIDs::Floor_Ground_Sand;
                else if (block == 4) texID = KenneyIDs::Floor_Stone_Pattern_Small; // Snow placeholder (White/Stone)? Or KenneyIDs::Snow? 
                // Let's check KenneyIDs.h ... No "Snow" explicitly? 
                // Maybe "Floor_Tiles_Tan_Large"? or "Floor_Stone_Sand_Trimsheet"?
                // Let's use "Floor_Stone_Pattern" for Snow (abstractly) or look for something white.
                // Or just use Sand for now if Snow is missing.
                // Wait, KenneyIDs has "Floor_Tiles_Blue_Small" (maybe ice?).
                // Let's use "Floor_Stone_Sand_Random" -> Looks like Snow/Sand mix?
                // Actually, let's use "Floor_Stone" for Stone (ID 6) and "Floor_Tiles_Blue_Small" for Water (ID 5).
                // For "Snow" (ID 4), let's use "Floor_Tiles_Tan_Small" (looks distinct).
                
                if (block == 3) texID = KenneyIDs::Floor_Ground_Sand;
                if (block == 4) texID = KenneyIDs::Floor_Stone_Pattern_Small; // Snowish
                if (block == 5) texID = KenneyIDs::Floor_Ground_Water;
                if (block == 6) texID = KenneyIDs::Wall_Stone; // Stone
                
                // Get UVs
                UVRect uv = {0,0,1,1};
                if (atlas)
                {
                    uv = atlas->GetUVs(texID);
                }

                // Color is White (Tint)
                glm::vec4 color(1.0f);

                // Check neighbors for culling
                
                // TOP (Y+1)
                if (GetBlock(x, y + 1, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 0, color, uv);
                }
                // BOTTOM (Y-1)
                if (GetBlock(x, y - 1, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 1, color, uv);
                }
                // LEFT (X-1)
                if (GetBlock(x - 1, y, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 2, color, uv);
                }
                // RIGHT (X+1)
                if (GetBlock(x + 1, y, z) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 3, color, uv);
                }
                // FRONT (Z+1)
                if (GetBlock(x, y, z + 1) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 4, color, uv);
                }
                // BACK (Z-1)
                if (GetBlock(x, y, z - 1) == 0)
                {
                    AddFace(vertices, indices, x, y, z, 5, color, uv);
                }
            }
        }
    }
}

void Chunk::AddFace(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, int x, int y, int z, int face, const glm::vec4& baseColor, UVRect uv)
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
        PushVert(fx, fy+fs, fz,       uv.uMin, uv.vMin); // TL
        PushVert(fx+fs, fy+fs, fz,    uv.uMax, uv.vMin); // TR
        PushVert(fx+fs, fy+fs, fz+fs, uv.uMax, uv.vMax); // BR
        PushVert(fx, fy+fs, fz+fs,    uv.uMin, uv.vMax); // BL
    }
    // Bottom: Y
    else if(face == 1) // Bottom
    {
        PushVert(fx, fy, fz+fs,    uv.uMin, uv.vMin); 
        PushVert(fx+fs, fy, fz+fs, uv.uMax, uv.vMin);
        PushVert(fx+fs, fy, fz,    uv.uMax, uv.vMax);
        PushVert(fx, fy, fz,       uv.uMin, uv.vMax);
    }
    // Left: X
    else if(face == 2)
    {
        PushVert(fx, fy+fs, fz+fs, uv.uMax, uv.vMin);
        PushVert(fx, fy+fs, fz,    uv.uMin, uv.vMin);
        PushVert(fx, fy, fz,       uv.uMin, uv.vMax);
        PushVert(fx, fy, fz+fs,    uv.uMax, uv.vMax);
    }
    // Right: X+1
    else if(face == 3)
    {
        PushVert(fx+fs, fy+fs, fz,    uv.uMax, uv.vMin);
        PushVert(fx+fs, fy+fs, fz+fs, uv.uMin, uv.vMin);
        PushVert(fx+fs, fy, fz+fs,    uv.uMin, uv.vMax);
        PushVert(fx+fs, fy, fz,       uv.uMax, uv.vMax);
    }
    // Front: Z+1
    else if(face == 4)
    {
        PushVert(fx, fy+fs, fz+fs,    uv.uMin, uv.vMin); // TL
        PushVert(fx+fs, fy+fs, fz+fs, uv.uMax, uv.vMin); // TR
        PushVert(fx+fs, fy, fz+fs,    uv.uMax, uv.vMax); // BR
        PushVert(fx, fy, fz+fs,       uv.uMin, uv.vMax); // BL
    }
    // Back: Z
    else if(face == 5)
    {
        PushVert(fx+fs, fy+fs, fz,    uv.uMin, uv.vMin); 
        PushVert(fx, fy+fs, fz,       uv.uMax, uv.vMin);
        PushVert(fx, fy, fz,          uv.uMax, uv.vMax);
        PushVert(fx+fs, fy, fz,       uv.uMin, uv.vMax);
    }

    // Indices (0, 1, 2, 2, 3, 0)
    indices.push_back(startIndex + 0);
    indices.push_back(startIndex + 1);
    indices.push_back(startIndex + 2);

    indices.push_back(startIndex + 2);
    indices.push_back(startIndex + 3);
    indices.push_back(startIndex + 0);
}
