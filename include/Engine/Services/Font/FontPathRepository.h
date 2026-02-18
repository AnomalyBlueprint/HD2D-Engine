#pragma once
#include <map>
#include <string>
#include <vector>
#include "Engine/Services/Data/PathRegistryService.h"

class FontPathRepository : public IPathRepository
{
public:
    virtual ~FontPathRepository() = default;

    void OnInitialize() override;
    void Clean() override { m_paths.clear(); }
    
    std::string GetPath(const std::string& fontName);
    
    // IPathRepository Interface
    std::string GetPath(int /*id*/) override { return ""; } // Fonts use string IDs

private:
    std::map<std::string, std::string> m_paths;
    void ScanDirectory(const std::string& path);
};
