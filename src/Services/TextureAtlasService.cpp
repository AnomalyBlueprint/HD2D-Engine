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

    // Estimate Atlas Size (Simple Grid Packer)
    int count = images.size();
    int cols = (int)std::ceil(std::sqrt((float)count));
    int rows = (int)std::ceil((float)count / cols);

    int tileW = maxWidth; 
    int tileH = maxHeight;

    m_atlasWidth = cols * tileW;
    m_atlasHeight = rows * tileH;
    
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
        float u0 = (float)xOff / m_atlasWidth;
        float v0 = (float)yOff / m_atlasHeight; 
        float u1 = (float)(xOff + img.w) / m_atlasWidth;
        float v1 = (float)(yOff + img.h) / m_atlasHeight; 
        
        m_uvMap[img.id] = { u0, v0, u1, v1 };

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
