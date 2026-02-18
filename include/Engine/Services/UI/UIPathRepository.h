#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "Engine/Services/Data/PathRegistryService.h"

// Scans UI directories and provides paths map
class UIPathRepository : public IPathRepository
{
public:
    virtual ~UIPathRepository() = default;
    
    void OnInitialize() override;
    void Clean() override { m_paths.clear(); }
    
    // Returns full path for a given filename (no extension)
    std::string GetPath(const std::string& name);

    // IPathRepository Interface
    std::string GetPath(int id) override { return ""; } // UI uses string IDs
    
    // Returns all found UI texture paths
    
    // Returns all found UI texture paths
    std::map<std::string, std::string> GetAllPaths();

private:
    std::map<std::string, std::string> m_paths;
    
    void ScanDirectory(const std::string& path);
};
