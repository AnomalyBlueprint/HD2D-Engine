#include "Engine/Services/BlockRegistryService.h"
#include <iostream>

void BlockRegistryService::RegisterBlock(uint8_t id, std::string name, bool transparent, bool collidable, 
                                         KenneyIDs top, KenneyIDs side, KenneyIDs bottom)
{
    BlockDefinition def;
    def.ID = id;
    def.Name = name;
    def.IsTransparent = transparent;
    def.IsCollidable = collidable;
    def.TextureTop = top;
    def.TextureSide = side;
    def.TextureBottom = bottom;
    m_blocks[id] = def;
}

void BlockRegistryService::Init()
{
    // Setup Default (Air/Error)
    m_defaultBlock = { 0, "Air", true, false, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt };

    // Register Standard Blocks
    // 1: Dirt
    RegisterBlock(1, "Dirt", false, true, 
        KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt);
    
    // 2: Grass
    RegisterBlock(2, "Grass", false, true, 
        KenneyIDs::Floor_Ground_Grass, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt);

    // 3: Sand
    RegisterBlock(3, "Sand", false, true, 
        KenneyIDs::Floor_Ground_Sand, KenneyIDs::Floor_Ground_Sand, KenneyIDs::Floor_Ground_Sand);

    // 4: Stone (using Wall_Brick_Small_Stone as substitute)
    RegisterBlock(4, "Stone", false, true, 
        KenneyIDs::Wall_Brick_Small_Stone, KenneyIDs::Wall_Brick_Small_Stone, KenneyIDs::Wall_Brick_Small_Stone);

    // 5: Water
    RegisterBlock(5, "Water", true, false, 
        KenneyIDs::Floor_Ground_Water, KenneyIDs::Floor_Ground_Water, KenneyIDs::Floor_Ground_Water);
    
    // 6: Bedrock
    RegisterBlock(6, "Bedrock", false, true, 
        KenneyIDs::Wall_Rock, KenneyIDs::Wall_Rock, KenneyIDs::Wall_Rock);
}

KenneyIDs BlockRegistryService::GetTextureID(int blockID, int faceID)
{
    // 0=Front, 1=Back, 2=Right, 3=Left, 4=Top, 5=Bottom
    if (m_blocks.find(blockID) != m_blocks.end())
    {
        const auto& def = m_blocks[blockID];
        if (faceID == 4) return def.TextureTop;
        if (faceID == 5) return def.TextureBottom;
        return def.TextureSide;
    }
    return m_defaultBlock.TextureTop;
}

const BlockDefinition& BlockRegistryService::GetBlockDef(int blockID)
{
    if (m_blocks.find(blockID) != m_blocks.end())
    {
        return m_blocks[blockID];
    }
    return m_defaultBlock;
}
