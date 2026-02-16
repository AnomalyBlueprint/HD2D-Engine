#pragma once
#include "Services/IBlockRegistryService.h"

/// <summary>
/// Concrete implementation of the Block Registry.
/// Maps Block IDs to Texture IDs (KenneyIDs).
/// </summary>
class BlockRegistryService : public IBlockRegistryService
{
public:
    void Init() override {}
    void Clean() override {}

    KenneyIDs GetTextureID(int blockID, int faceID) override;
};
