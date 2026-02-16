#pragma once
#include "Engine/Services/IBlockRegistryService.h"
#include <unordered_map>

/// <summary>
/// Concrete implementation of the Block Registry.
/// Maps Block IDs to Texture IDs (KenneyIDs).
/// </summary>
class BlockRegistryService : public IBlockRegistryService
{
private:
    std::unordered_map<int, BlockDefinition> m_blocks;
    BlockDefinition m_defaultBlock; // Fallback (Air or error block)

    void RegisterBlock(uint8_t id, std::string name, bool transparent, bool collidable, 
                       KenneyIDs top, KenneyIDs side, KenneyIDs bottom);

public:
    void Init() override;
    void Clean() override {}

    KenneyIDs GetTextureID(int blockID, int faceID) override;
    const BlockDefinition& GetBlockDef(int blockID) override;
};
