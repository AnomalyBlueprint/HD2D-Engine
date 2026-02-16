#include "Engine/Services/BlockRegistryService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"

void BlockRegistryService::OnInitialize()
{
    auto logger = ServiceLocator::Get().GetService<ILoggerService>();
    if(logger) logger->Log("Initializing Block Registry...");

    // Register Standard Blocks
    // ID 1: Dirt
    RegisterBlock(1, { KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, false, true });
    // ID 2: Grass
    RegisterBlock(2, { KenneyIDs::Floor_Ground_Grass, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, false, true });
    // ID 3: Stone
    RegisterBlock(3, { KenneyIDs::Wall_Stone, KenneyIDs::Wall_Stone, KenneyIDs::Wall_Stone, false, true });
    // ID 4: Water
    RegisterBlock(4, { KenneyIDs::Floor_Ground_Water, KenneyIDs::Floor_Ground_Water, KenneyIDs::Floor_Ground_Water, true, false });
}

void BlockRegistryService::RegisterBlock(uint8_t id, BlockDef def)
{
    m_blocks[id] = def;
}

const BlockDef& BlockRegistryService::GetBlock(uint8_t id)
{
    if (m_blocks.find(id) != m_blocks.end()) {
        return m_blocks[id];
    }
    static BlockDef air = { KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, KenneyIDs::Floor_Ground_Dirt, true, false };
    return air;
}
