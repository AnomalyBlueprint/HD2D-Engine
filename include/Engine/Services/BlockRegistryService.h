#pragma once
#include "Engine/Services/IBlockRegistryService.h"
#include <unordered_map>

/// <summary>
/// Concrete implementation of the Block Registry.
/// </summary>
class BlockRegistryService : public IBlockRegistryService
{
private:
    std::unordered_map<uint8_t, BlockDef> m_blocks;
   
    void RegisterBlock(uint8_t id, BlockDef def);

public:
    void Init() override;
    void Clean() override {}

    const BlockDef& GetBlock(uint8_t id) override;
};
