#include "Engine/Services/UIService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/IFontService.h"
#include <fstream>
#include <iostream>
#include <sstream>

UIService::UIService() {}

UIService::~UIService() {}

void UIService::OnInitialize()
{
}

void UIService::Clean()
{
    m_layouts.clear();
}

void UIService::LoadLayouts(const std::string& path)
{
    // Ensure we are loading the correct file (sanity check or override)
    // The user explicit request: "Ensure explicit load of assets/ui/ui_layouts.json"
    // Since argument 'path' is passed, we assume caller sends correct path, 
    // but we can log it clearly.
    
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[UIService] Failed to open layout file: " << path << std::endl;
        return;
    }

    nlohmann::json j;
    file >> j;

    if (j.contains("settings"))
    {
        m_width = j["settings"].value("width", 1280);
        m_height = j["settings"].value("height", 720);
    }

    if (j.contains("scenes"))
    {
        for (auto& [sceneName, elements] : j["scenes"].items())
        {
            std::vector<UIElement> uiElements;
            for (const auto& el : elements)
            {
                UIElement uiEl;
                uiEl.id = el.value("id", "");
                uiEl.type = el.value("type", "CONTAINER");

                if (el.contains("geometry"))
                {
                    const auto& geo = el["geometry"];
                    uiEl.geometry.x = geo.value("x", 0.0f);
                    uiEl.geometry.y = geo.value("y", 0.0f);
                    uiEl.geometry.w = geo.value("w", 0.0f);
                    uiEl.geometry.h = geo.value("h", 0.0f);
                }

                if (el.contains("style"))
                {
                    const auto& style = el["style"];
                    if (style.contains("bg")) uiEl.style.bg = HexToVec4(style["bg"]);
                    if (style.contains("border")) uiEl.style.border = HexToVec4(style["border"]);
                    if (style.contains("color")) uiEl.style.color = HexToVec4(style["color"]);
                    if (style.contains("image")) uiEl.style.image = style.value("image", "");
                }

                if (el.contains("properties"))
                {
                    const auto& props = el["properties"];
                    uiEl.properties.text = props.value("text", "");
                    uiEl.properties.fontSize = props.value("fontSize", 12);
                    uiEl.properties.align = props.value("align", "left");
                    uiEl.properties.actionId = props.value("actionId", "");
                    uiEl.properties.checked = props.value("checked", false);
                    uiEl.properties.label = props.value("label", "");
                }

                uiElements.push_back(uiEl);
            }
            m_layouts[sceneName] = uiElements;
        }
    }
    
    std::cout << "[INFO] UI Layouts loaded from " << path << std::endl;
    std::cout << "[UIService] Loaded " << m_layouts.size() << " scenes." << std::endl;
    // Default to main_menu if available
    if (m_layouts.find("main_menu") != m_layouts.end())
    {
        SetScene("main_menu");
    }
}

void UIService::SetScene(const std::string& sceneName)
{
    if (m_layouts.find(sceneName) != m_layouts.end())
    {
        m_activeScene = sceneName;
    }
    else
    {
        std::cerr << "[UIService] Scene not found: " << sceneName << std::endl;
    }
}

void UIService::GetScreenSize(int& w, int& h)
{
    w = m_width;
    h = m_height;
}

glm::vec4 UIService::HexToVec4(const std::string& hex)
{
    if (hex.empty() || hex == "transparent") return glm::vec4(0.0f);

    std::string h = hex;
    if (h[0] == '#') h = h.substr(1);

    // Handle alpha if present (RRGGBBAA) or just RRGGBB
    float r = 0, g = 0, b = 0, a = 1.0f;
    
    if (h.length() >= 6)
    {
        std::stringstream ss;
        ss << std::hex << h.substr(0, 2);
        int ri; ss >> ri; r = ri / 255.0f;
        
        ss.clear(); ss.str("");
        ss << std::hex << h.substr(2, 2);
        int gi; ss >> gi; g = gi / 255.0f;
        
        ss.clear(); ss.str("");
        ss << std::hex << h.substr(4, 2);
        int bi; ss >> bi; b = bi / 255.0f;
    }
    
    if (h.length() == 8)
    {
        std::stringstream ss;
        ss << std::hex << h.substr(6, 2);
        int ai; ss >> ai; a = ai / 255.0f;
    }

    return glm::vec4(r, g, b, a);
}

void UIService::Render(RenderService* renderer)
{
    if (m_activeScene.empty() || m_layouts.find(m_activeScene) == m_layouts.end()) return;

    void* atlasServiceVoid = ServiceLocator::Get().GetService<TextureAtlasService>().get();
    TextureAtlasService* atlas = static_cast<TextureAtlasService*>(atlasServiceVoid);
    
    // Bind Atlas once for batching (if we had batching across DrawSprite calls that shared texture)
    // For now DrawSprite binds texture individually if different. 
    // Ideally we pass atlas ID.
    
    const auto& elements = m_layouts[m_activeScene];
    for (const auto& el : elements)
    {
        // 1. Draw Background (if bg color is not transparent)
        // 1. Draw Background or Image
        if (!el.style.image.empty() && atlas)
        {
             Sprite imgSprite;
             imgSprite.Position = glm::vec2(el.geometry.x, el.geometry.y);
             imgSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
             imgSprite.Color = glm::vec4(1.0f); // White tint for texture
             imgSprite.TextureID = atlas->GetTextureID();
             
             UVRect uvs = atlas->GetUVs(el.style.image);
             imgSprite.MinUV = glm::vec2(uvs.uMin, uvs.vMin);
             imgSprite.MaxUV = glm::vec2(uvs.uMax, uvs.vMax);
             
             renderer->DrawSprite(imgSprite);
        }
        else if (el.style.bg.a > 0.0f)
        {
            Sprite bgSprite;
            bgSprite.Position = glm::vec2(el.geometry.x, el.geometry.y);
            bgSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
            bgSprite.Color = el.style.bg;
            bgSprite.TextureID = 0; // Solid color
            renderer->DrawSprite(bgSprite);
        }

        // 2. Draw Border (Simulated with inner/outer rect or lines? For now, skip or simple border)
        // If we want a border, we might draw a slightly larger rect behind, or using lines
        // For simplicity, reusing Sprite for filled rects only for now.
        
        
        // 3. Draw Text / Label
        // TODO: Implement FontService (Load .ttf from assets/fonts/)
        // For now, render 'LABEL' elements as colored rectangles so we can verify the layout.
        // The user said: "For 'LABEL', prepare a placeholder or use the sprite system if text-rendering isn't ready yet."
        // I will draw a small placeholder rect or debug line if it's a label? 
        // Or just nothing if we can't see text. 
        // Actually, let's draw a placeholder colored box for text areas if they have no bg?
        if (el.type == "LABEL" || !el.properties.text.empty())
        {
             auto font = ServiceLocator::Get().GetService<IFontService>();
             if (font) {
                 // Calculate position (Center or Left aligned?)
                 // JSON says "align": "center" or "left".
                 float x = el.geometry.x;
                 float y = el.geometry.y + el.geometry.h / 2 + 10; // Approx vertical center offset
                 
                 // If centered, offset x
                 float fontSizeLoaded = 32.0f; // Keep consistent with FontService
                 float scale = (el.properties.fontSize > 0) ? (el.properties.fontSize / fontSizeLoaded) : 1.0f;

                 if (el.properties.align == "center") {
                     float textW = font->GetTextWidth(el.properties.text, scale);
                     x += (el.geometry.w - textW) / 2;
                 } else { // padding
                     x += 10;
                 }
                 
                 // Render Text
                 // Use text color from style, or default white/black
                 glm::vec4 textColor = el.style.color;
                 if (textColor.a == 0.0f) textColor = glm::vec4(1.0f);
                 
                 font->RenderText(renderer, el.properties.text, x, y, scale, textColor);
             }
        }
    }
}
