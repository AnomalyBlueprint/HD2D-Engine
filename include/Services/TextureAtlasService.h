#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Data/KenneyIDs.h"
#include <glm/glm.hpp>

#include "Services/IService.h"

// Forward declarations
class KenneyPathRepository;

struct UVRect {
    float uMin, vMin;
    float uMax, vMax;
};

/// <summary>
/// Service that packs individual textures into a single Texture Atlas.
/// Reduces draw calls by allowing batching of sprites sharing the same atlas.
/// </summary>
class TextureAtlasService : public IService
{
public:
    TextureAtlasService();
    ~TextureAtlasService();

    void Init() override {} 
    void Update() override {}
    void Clean() override {}

    /// <summary>
    /// Loads all textures from the repository and creates the Atlas texture.
    /// </summary>
    void LoadAtlas(std::shared_ptr<KenneyPathRepository> repo);

    /// <summary>
    /// Returns the OpenGL Texture ID of the Atlas.
    /// </summary>
    unsigned int GetTextureID() const { return m_atlasTextureID; }

    /// <summary>
    /// Returns UV coordinates for a specific texture ID within the atlas.
    /// </summary>
    UVRect GetUVs(KenneyIDs id);

private:
    unsigned int m_atlasTextureID = 0;
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    
    std::map<KenneyIDs, UVRect> m_uvMap;

    void PackTextures(const std::vector<std::pair<KenneyIDs, std::string>>& textures);
};
