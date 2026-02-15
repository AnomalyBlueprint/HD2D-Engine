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

class TextureAtlasService : public IService
{
public:
    TextureAtlasService();
    ~TextureAtlasService();

    // IService Implementation
    void Init() override {} // Empty, as we need custom init with Repo
    void Update() override {}
    void Clean() override {}

    // Loads all textures from the repository and creates the Atlas
    void LoadAtlas(std::shared_ptr<KenneyPathRepository> repo);

    // Returns the OpenGL Texture ID of the Atlas
    unsigned int GetTextureID() const { return m_atlasTextureID; }

    // Returns UVs for a specific KenneyID
    UVRect GetUVs(KenneyIDs id);

private:
    unsigned int m_atlasTextureID = 0;
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    
    // Map of Enum -> UV Rect
    std::map<KenneyIDs, UVRect> m_uvMap;

    void PackTextures(const std::vector<std::pair<KenneyIDs, std::string>>& textures);
};
