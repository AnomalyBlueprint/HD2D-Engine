#pragma once
#include "Engine/Services/IPathRepository.h"
#include "Engine/Data/KenneyIDs.h"
#include <unordered_map>

/// <summary>
/// Implementation of IPathRepository for Kenney assets.
/// </summary>
class KenneyPathRepository : public IPathRepository
{
public:
protected:
    void OnInitialize() override;
public:
    void Clean() override { m_paths.clear(); }
    
    std::string GetPath(int assetID) override;

private:
   std::unordered_map<KenneyIDs, std::string> m_paths;
};
