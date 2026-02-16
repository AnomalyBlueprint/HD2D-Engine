#pragma once
#include "Services/IPathRepository.h"
#include "Data/KenneyIDs.h"
#include <unordered_map>

/// <summary>
/// Implementation of IPathRepository for Kenney assets.
/// </summary>
class KenneyPathRepository : public IPathRepository
{
public:
    void Init() override;
    void Clean() override { m_paths.clear(); }
    
    std::string GetPath(int assetID) override;

private:
   std::unordered_map<KenneyIDs, std::string> m_paths;
};
