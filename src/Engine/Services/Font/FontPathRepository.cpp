#include "Engine/Services/Font/FontPathRepository.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/ILoggerService.h"
#include <filesystem>

namespace fs = std::filesystem;

void FontPathRepository::OnInitialize()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    if(log) log->Log("Initializing Font Path Repository...");
    
    ScanDirectory("assets/fonts/kenney_kenney-fonts/Fonts/");
}

void FontPathRepository::ScanDirectory(const std::string& path)
{
    if (!fs::exists(path)) return;

    for (const auto& entry : fs::recursive_directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
            if (entry.path().extension() == ".ttf" || entry.path().extension() == ".otf")
            {
                std::string name = entry.path().stem().string();
                m_paths[name] = entry.path().string();
            }
        }
    }
}

std::string FontPathRepository::GetPath(const std::string& fontName)
{
    if (m_paths.find(fontName) != m_paths.end())
        return m_paths[fontName];
    return "";
}
