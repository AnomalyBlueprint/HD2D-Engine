#include "Services/TextureAtlasService.h"
#include "Services/KenneyPathRepository.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>
#include <algorithm>
#include <cmath>

// Include stb_image locally if not already available in a compiled object?
// Usually defined in RenderService or similar. To be safe, we might need it here or rely on the one in RenderService.
// Since STB_IMAGE_IMPLEMENTATION is likely in OpenGLRenderService.cpp, we just need the header.
#include <vendor/stb_image.h>
#include <GL/glew.h> 

TextureAtlasService::TextureAtlasService()
{
}

TextureAtlasService::~TextureAtlasService()
{
    if (m_atlasTextureID != 0)
    {
        glDeleteTextures(1, &m_atlasTextureID);
    }
}

void TextureAtlasService::LoadAtlas(std::shared_ptr<KenneyPathRepository> repo)
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    log->Log("Initializing Texture Atlas...");

    // 1. Gather all paths
    std::vector<std::pair<KenneyIDs, std::string>> texturesToLoad;
    
    // Iterate over all enum values? Hard to iterate enum class.
    // We can iterate the 'm_paths' map from the repository if we expose it, 
    // or just iterate a known range if KenneyIDs is sequential.
    // Better: Add a method to KenneyPathRepository to "GetAllPaths".
    // Alternatively, we loop from 1 to 123 (based on KenneyIDs.h count).
    
    for (int i = 1; i <= 123; i++) // Hacky but works for now given static Enum
    {
        KenneyIDs id = (KenneyIDs)i;
        std::string path = repo->GetPath(i);
        if (!path.empty())
        {
            texturesToLoad.push_back({id, path});
        }
    }

    if (texturesToLoad.empty())
    {
        log->LogError("No textures found to pack into Atlas!");
        return;
    }

    PackTextures(texturesToLoad);
}

void TextureAtlasService::PackTextures(const std::vector<std::pair<KenneyIDs, std::string>>& textures)
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();

    // Configuration
    // const int PADDING = 0; // Unused
    
    // We assume tiles are somewhat uniform (e.g. 64x64 or 128x128).
    // Let's load the first one to check size.
    // int tWidth, tHeight, tChannels; // Unused 
    
    // Temporary buffer to hold all loaded image data
    struct LoadedImage {
        KenneyIDs id;
        unsigned char* data;
        int w, h;
    };
    std::vector<LoadedImage> images;

    int maxWidth = 0;
    int maxHeight = 0;

    // Load ALL images
    for (const auto& pair : textures)
    {
        int w, h, c;
        // Don't flip on load for atlas packing unless we write bottom-up.
        // OpenGL 0,0 is Bottom-Left. 
        // If we pack grid Top-Down, we might want flipped?
        // Let's keep consistency with RenderService: stbi_set_flip_vertically_on_load(true);
        // BUT if we use glTexSubImage2D, row order matters.
        // Let's use false for Atlas assembly in memory (Top-Down), then upload.
        stbi_set_flip_vertically_on_load(false); 
        
        unsigned char* data = stbi_load(pair.second.c_str(), &w, &h, &c, 4); // Force 4 channels (RGBA)
        if (data)
        {
            images.push_back({pair.first, data, w, h});
            maxWidth = std::max(maxWidth, w);
            maxHeight = std::max(maxHeight, h);
        }
        else
        {
            log->LogError("Failed to load for Atlas: " + pair.second);
        }
    }

    if (images.empty()) return;

    // Estimate Atlas Size
    // Simple Grid Packer
    int count = images.size();
    // Area needed ~= count * maxW * maxH
    // Ideally square atlas. Sqrt(count)
    int cols = (int)std::ceil(std::sqrt((float)count));
    int rows = (int)std::ceil((float)count / cols);

    int tileW = maxWidth; // Force uniform grid based on max size
    int tileH = maxHeight;

    m_atlasWidth = cols * tileW;
    m_atlasHeight = rows * tileH;
    
    // Round up to POT (Power of Two) not strictly required but good practice.
    // Let's stick to calculated size for now to avoid wasted space.

    // Allocate Atlas Buffer (Zero initialized)
    std::vector<unsigned char> atlasData(m_atlasWidth * m_atlasHeight * 4, 0);

    // Pack
    int idx = 0;
    for (const auto& img : images)
    {
        int col = idx % cols;
        int row = idx / cols;

        int xOff = col * tileW;
        int yOff = row * tileH;

        // Copy pixels row by row
        for (int y = 0; y < img.h; y++)
        {
            for (int x = 0; x < img.w; x++)
            {
                int atlasIndex = ((yOff + y) * m_atlasWidth + (xOff + x)) * 4;
                int imgIndex = (y * img.w + x) * 4;

                atlasData[atlasIndex + 0] = img.data[imgIndex + 0];
                atlasData[atlasIndex + 1] = img.data[imgIndex + 1];
                atlasData[atlasIndex + 2] = img.data[imgIndex + 2];
                atlasData[atlasIndex + 3] = img.data[imgIndex + 3];
            }
        }

        // Calculate UVs
        // Note: UVs are 0..1. OpenGL 0,0 is Bottom-Left.
        // Our buffer is Top-Down (because we didn't flip).
        // So Row 0 is at the TOP of the texture?
        // If we upload this buffer as is, OpenGL treats the first byte as Bottom-Left?
        // NO, OpenGL defaults to Bottom-Left origin for texture coordinates, 
        // but glTexImage2D expects data starting from Bottom Row if it's standard.
        // Actually: "The first element corresponds to the lower left corner of the texture image"
        // IF we didn't flip. STB loads Top-Down by default.
        // So first byte is Top-Left pixel of image.
        // If we upload Top-Left data to OpenGL, it will appear upside down unless we flip UVs or data.
        // Solution: Calculate UVs assuming Top-Down origin? 
        // OpenGL v=0 is Bottom, v=1 is Top.
        // If we upload Top-down data:
        // Byte 0 (Image Top) -> OpenGL (0,0) (Bottom).
        // So the image is Upside Down.
        // To fix: v = 1 - v;
        
        // Let's compute normalized coords for the slot:
        float u0 = (float)xOff / m_atlasWidth;
        float v0 = (float)yOff / m_atlasHeight; // Top of slot
        float u1 = (float)(xOff + img.w) / m_atlasWidth;
        float v1 = (float)(yOff + img.h) / m_atlasHeight; // Bottom of slot

        // Since the texture will be upside down in GL, 
        // The "Top" of the image (yOff) will act as V=0 (Bottom) in GL.
        // So UV (0,0) is Image Top-Left.
        // This effectively flips the V axis for rendering if we map 0..1 normally.
        // Wait, standard mapping:
        // Quad Top: V=1. Quad Bottom: V=0.
        // Texture in Memory: Row 0 = Image Top.
        // Texture in GL: V=0 = Image Top. V=1 = Image Bottom.
        // So Image is Upside Down.
        // To display correctly: 
        // Quad Top (V=1) should map to Image Top (V=0 in GL).
        // Quad Bottom (V=0) should map to Image Bottom (V=1 in GL).
        // So we need to INVERT V in the UVs.
        
        // Let's store UVs matching the logical Top/Bottom of the image in the Atlas.
        // Image Top (visual) is at `yOff`. In GL coords (0..1), this is `yOff / H`.
        // Image Bottom (visual) is at `yOff + H`. In GL coords, this is `(yOff+H)/H`.
        
        // Quad Top needs Image Top.
        // UV.vMax = yOff / H? No wait.
        
        // Let's stick to: We upload Top-Down data.
        // GL (0,0) contains Top-Left pixel.
        // GL (1,1) contains Bottom-Right pixel. (Wait, row order...)
        
        // Let's just implement `1.0 - v` logic in UVs.
        // Standard Grid:
        // uMin = x / W
        // uMax = (x+w) / W
        // vMin = y / H   <-- This is "Top" in memory
        // vMax = (y+h) / H <-- This is "Bottom" in memory
        
        // Storing these directly:
        // Usage on Quad:
        // BL: (uMin, vMax) -> (Left, Bottom in mem)
        // TL: (uMin, vMin) -> (Left, Top in mem) 
        // ...
        // If we map TL vertex to vMin, it pulls from "Top in mem".
        // Since "Top in mem" is at GL 0.0 (Bottom), TL vertex gets Top pixel.
        // But TL vertex is at Top of screen.
        // Rendered: Top of screen gets "Top in mem" (mapped to GL Bottom).
        // So visually: Image Top appears at Top of Screen.
        // So Image is UPRIGHT!
        
        // Wait. If GL(0,0) is BL, and we put Top-Pixel at Byte 0...
        // Then BL of mesh samples GL(0,0) -> gets Top-Pixel.
        // So Mesh Bottom shows Image Top. -> FLIPPED.
        
        // Okay, simpler solution: Flip the whole atlas in memory before upload.
        // Or flip rows of each image during copy? No, copy to specific rects.
        // Let's Flip the final Atlas buffer.
        
        UVRect uv;
        uv.uMin = u0;
        uv.uMax = u1;
        // Standard UVs assuming texture is upright
        uv.vMin = v0; // Top? No, vMin usually 0 (Bottom).
        uv.vMax = v1;
        
        // Correct Approach:
        // Just upload as is (flipped).
        // Adjust UVs:
        // We want Image Top (at v0 in memory) to be at Quad Top (V=1).
        // We want Image Bottom (at v1 in memory) to be at Quad Bottom (V=0).
        // So UVs for Quad:
        // TL Vert: V = v0 (Memory location of Top) -> Wait, if Memory 0 is GL 0...
        // TL Vert (V=1?) No, we assign specific UVs to verts.
        // Vertex.texCoord = ?
        // TL: (uMin, vMin) ?? 
        // If we set TL UV to vMin, and vMin points to 0.0 (Memory Top), then TL displays Memory Top.
        // Visually: Screen Top displays Image Top. Correct!
        // So we just use direct mapping.
        
        // BUT `Chunk.cpp` currently generates UVs:
        // TL: (0, 1), BL: (0, 0)
        // It assumes V=1 is Top.
        // If we return UV rect, we need to say what is Top and Bottom.
        
        // Let's store:
        // uMin, vMin (Top-Left in Atlas)
        // uMax, vMax (Bottom-Right in Atlas)
        
        // In Chunk:
        // TL: uMin, vMin (if vMin is close to 0)
        // BL: uMin, vMax (if vMax is close to 1)
        
        // Wait, if vMin is 0 (Top of memory), and GL treats 0 as Bottom...
        // TL displays GL 0.0 (Bottom of Texture).
        // But our Texture at 0.0 contains the Image Top.
        // So Screen Top displays Image Top.
        // Correct.
        
        // So:
        // uMin = x / W
        // uMax = (x+w) / W
        // vMin = y / H
        // vMax = (y+h) / H
        
        m_uvMap[img.id] = { u0, v0, u1, v1 };

        // Free sub-image
        stbi_image_free(img.data);
        
        idx++;
    }
    
    // Upload Atlas
    glGenTextures(1, &m_atlasTextureID);
    glBindTexture(GL_TEXTURE_2D, m_atlasTextureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_atlasWidth, m_atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());

    log->Log("Atlas Created: " + std::to_string(m_atlasWidth) + "x" + std::to_string(m_atlasHeight) + ", Textures: " + std::to_string(count));
    
    // Debug: Save atlas? No.
}

UVRect TextureAtlasService::GetUVs(KenneyIDs id)
{
    if (m_uvMap.find(id) != m_uvMap.end())
    {
        return m_uvMap[id];
    }
    return {0,0,1,1}; // Fallback: Full texture?
}
