#include "Engine/Services/UIPathRepository.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void UIPathRepository::OnInitialize()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    if(log) log->Log("Initializing UI Path Repository...");
    
    // Scan standard UI folders
    ScanDirectory("assets/ui/kenney_ui-pack-pixel-adventure/Tiles/Large tiles/Thin outline/");
    ScanDirectory("assets/ui/kenney_fantasy-ui-borders/");
    
    // Manual registration for specific icons in root assets/ui
    m_paths["expand"] = "assets/ui/expand.png";
    m_paths["contract"] = "assets/ui/contract.png";
    m_paths["plain-arrow-down"] = "assets/ui/plain-arrow-down.png";
    m_paths["infinity"] = "assets/ui/infinity.png";
}

void UIPathRepository::ScanDirectory(const std::string& path)
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    
    if (!fs::exists(path))
    {
        if(log) log->LogWarning("UI Path not found: " + path);
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
            auto path = entry.path();
            std::string name = path.stem().string();
            
            // Filter out large sample/preview images
            if (name == "Sample" || name == "Preview") continue;
            
            if (path.extension() == ".png")
            {
                // If collision, we overwrite or warn. Here we overwrite.
                m_paths[name] = path.string();
            }
        }
    }
}

std::string UIPathRepository::GetPath(const std::string& name)
{
    if (m_paths.find(name) != m_paths.end())
        return m_paths[name];
    return "";
}

std::map<std::string, std::string> UIPathRepository::GetAllPaths()
{
    return m_paths;
}
