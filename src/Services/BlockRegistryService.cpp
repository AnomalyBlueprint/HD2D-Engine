#include "Services/BlockRegistryService.h"

KenneyIDs BlockRegistryService::GetTextureID(int blockID, int faceID)
{
    // 0=Front, 1=Back, 2=Right, 3=Left, 4=Top, 5=Bottom

    switch (blockID)
    {
    case 1: // Dirt
        return KenneyIDs::Floor_Ground_Dirt;
    
    case 2: // Grass
        if (faceID == 4) return KenneyIDs::Floor_Ground_Grass; // Top
        return KenneyIDs::Floor_Ground_Dirt; // Sides/Bottom
    
    case 3: // Sand
        return KenneyIDs::Floor_Ground_Sand;
    
    case 5: // Water
        return KenneyIDs::Floor_Ground_Water;
    
    default:
        return KenneyIDs::Floor_Ground_Dirt;
    }
}
