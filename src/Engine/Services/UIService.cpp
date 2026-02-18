#include "Engine/Services/UIService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/TextureAtlasService.h"
#include "Engine/Services/IFontService.h"
#include "Engine/Services/IInputService.h"
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
                    if (style.contains("rotation")) uiEl.style.rotation = style.value("rotation", 0.0f);
                }

                if (el.contains("properties"))
                {
                    const auto& props = el["properties"];
                    uiEl.properties.text = props.value("text", "");
                    uiEl.properties.fontSize = props.value("fontSize", 12);
                    uiEl.properties.fontName = props.value("fontName", "");
                    uiEl.properties.align = props.value("align", "left");
                    uiEl.properties.actionId = props.value("actionId", "");
                    uiEl.properties.checked = props.value("checked", false);
                    uiEl.properties.label = props.value("label", "");
                    uiEl.properties.min = props.value("min", 0);
                    uiEl.properties.max = props.value("max", 100);
                    uiEl.properties.value = props.value("value", 50);
                    uiEl.properties.layoutMode = props.value("layoutMode", "free");
                    uiEl.properties.prefabId = props.value("prefabId", "");
                    uiEl.properties.prefabHostId = props.value("prefabHostId", "");
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

void UIService::SetElementText(const std::string& sceneName, const std::string& elementId, const std::string& text)
{
    if (m_layouts.find(sceneName) == m_layouts.end()) return;

    for (auto& el : m_layouts[sceneName])
    {
        if (el.id == elementId)
        {
            el.properties.text = text;
            return;
        }
    }
}

void UIService::SetElementTexture(const std::string& sceneName, const std::string& elementId, unsigned int textureId)
{
    if (m_layouts.find(sceneName) == m_layouts.end()) return;

    for (auto& el : m_layouts[sceneName])
    {
        if (el.id == elementId)
        {
            el.style.textureId = textureId;
            return;
        }
    }
}

void UIService::SetElementUVs(const std::string& sceneName, const std::string& elementId, const glm::vec2& uvMin, const glm::vec2& uvMax)
{
    if (m_layouts.find(sceneName) == m_layouts.end()) return;

    for (auto& el : m_layouts[sceneName])
    {
        if (el.id == elementId)
        {
            el.style.uvMin = uvMin;
            el.style.uvMax = uvMax;
            return;
        }
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
        // Skip Prefab Templates (they are cloned, not rendered directly)
        if (!el.properties.prefabId.empty()) continue;

        // 1. Draw Background or Image
        if (!el.style.image.empty() && atlas)
        {
             Sprite imgSprite;
             // Convert Top-Left layout to Center position for DrawSprite
             imgSprite.Position = glm::vec2(
                 el.geometry.x + el.geometry.w / 2.0f, 
                 el.geometry.y + el.geometry.h / 2.0f
             );
             imgSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
             imgSprite.Color = glm::vec4(1.0f); // White tint for texture
             imgSprite.TextureID = atlas->GetTextureID();
             imgSprite.Rotation = el.style.rotation;
             
             UVRect uvs = atlas->GetUVs(el.style.image);
             imgSprite.MinUV = glm::vec2(uvs.uMin, uvs.vMin);
             imgSprite.MaxUV = glm::vec2(uvs.uMax, uvs.vMax);
             
             renderer->DrawSprite(imgSprite);
        }
        else if (el.style.bg.a > 0.0f)
        {
            Sprite bgSprite;
            // Convert Top-Left layout to Center position for DrawSprite
            bgSprite.Position = glm::vec2(
                el.geometry.x + el.geometry.w / 2.0f, 
                el.geometry.y + el.geometry.h / 2.0f
            );
            bgSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
            bgSprite.Color = el.style.bg;
            bgSprite.TextureID = 0; // Solid color
            renderer->DrawSprite(bgSprite);
        }
        else if (el.style.textureId != 0)
        {
            Sprite texSprite;
            texSprite.Position = glm::vec2(
                el.geometry.x + el.geometry.w / 2.0f, 
                el.geometry.y + el.geometry.h / 2.0f
            );
            texSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
            texSprite.Color = glm::vec4(1.0f); // White for texture
            texSprite.TextureID = el.style.textureId;
            texSprite.MinUV = el.style.uvMin;
            texSprite.MaxUV = el.style.uvMax;
            texSprite.Rotation = el.style.rotation;
            renderer->DrawSprite(texSprite);
        }

        // Border Rendering
        if (el.style.border.a > 0.0f)
        {
            float thickness = 2.0f;
            glm::vec4 borderColor = el.style.border;
            
            // Top
            Sprite top;
            top.Position = glm::vec2(el.geometry.x + el.geometry.w / 2.0f, el.geometry.y + thickness / 2.0f);
            top.Size = glm::vec2(el.geometry.w, thickness);
            top.Color = borderColor;
            top.TextureID = 0;
            renderer->DrawSprite(top);

            // Bottom
            Sprite bot;
            bot.Position = glm::vec2(el.geometry.x + el.geometry.w / 2.0f, el.geometry.y + el.geometry.h - thickness / 2.0f);
            bot.Size = glm::vec2(el.geometry.w, thickness);
            bot.Color = borderColor;
            bot.TextureID = 0;
            renderer->DrawSprite(bot);

            // Left
            Sprite left;
            left.Position = glm::vec2(el.geometry.x + thickness / 2.0f, el.geometry.y + el.geometry.h / 2.0f);
            left.Size = glm::vec2(thickness, el.geometry.h);
            left.Color = borderColor;
            left.TextureID = 0;
            renderer->DrawSprite(left);

            // Right
            Sprite right;
            right.Position = glm::vec2(el.geometry.x + el.geometry.w - thickness / 2.0f, el.geometry.y + el.geometry.h / 2.0f);
            right.Size = glm::vec2(thickness, el.geometry.h);
            right.Color = borderColor;
            right.TextureID = 0;
            renderer->DrawSprite(right);
        }

        // 2. Draw Specific Widgets
        if (el.type == "SLIDER")
        {
            // Draw Track
            Sprite trackSprite;
            trackSprite.Position = glm::vec2(el.geometry.x + el.geometry.w / 2.0f, el.geometry.y + el.geometry.h / 2.0f);
            trackSprite.Size = glm::vec2(el.geometry.w, el.geometry.h / 4.0f); // Thin track
            trackSprite.Color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f); // Dark Gray
            trackSprite.TextureID = 0;
            renderer->DrawSprite(trackSprite);

            // Draw Knob
            float range = (float)(el.properties.max - el.properties.min);
            float pct = (range > 0) ? ((float)(el.properties.value - el.properties.min) / range) : 0.0f;
            float knobX = el.geometry.x + (el.geometry.w * pct);
            
            Sprite knobSprite;
            knobSprite.Position = glm::vec2(knobX, el.geometry.y + el.geometry.h / 2.0f);
            knobSprite.Size = glm::vec2(10.0f, el.geometry.h); 
            knobSprite.Color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f); // Green
            knobSprite.TextureID = 0;
            renderer->DrawSprite(knobSprite);
        }
        else if (el.type == "CHECKBOX")
        {
             // Draw Box
             Sprite boxSprite;
             boxSprite.Position = glm::vec2(el.geometry.x + el.geometry.w / 2.0f, el.geometry.y + el.geometry.h / 2.0f);
             boxSprite.Size = glm::vec2(el.geometry.w, el.geometry.h);
             boxSprite.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
             boxSprite.TextureID = 0;
             renderer->DrawSprite(boxSprite);

             if (el.properties.checked)
             {
                 Sprite checkSprite;
                 checkSprite.Position = boxSprite.Position;
                 checkSprite.Size = boxSprite.Size * 0.6f;
                 checkSprite.Color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
                 checkSprite.TextureID = 0;
                 renderer->DrawSprite(checkSprite);
             }
             
             // Draw Label if exists
             if (!el.properties.label.empty())
             {
                 auto font = ServiceLocator::Get().GetService<IFontService>();
                 if (font) {
                     font->RenderText(renderer, el.properties.label, 
                         el.geometry.x + el.geometry.w + 10, 
                         el.geometry.y + el.geometry.h / 2 + 5, 
                         0.5f, glm::vec4(1.0f), el.properties.fontName);
                 }
             }
        }
        
        // 3. Draw Text / Content
        if (el.type == "LABEL" || el.type == "BUTTON")
        {
             if (!el.properties.text.empty())
             {
                 auto font = ServiceLocator::Get().GetService<IFontService>();
                 if (font) {
                     float x = el.geometry.x;
                     float y = el.geometry.y + el.geometry.h / 2 + 10; // Approx vertical center offset
                     
                     float fontSizeLoaded = 32.0f; // Keep consistent with FontService
                     float scale = (el.properties.fontSize > 0) ? (el.properties.fontSize / fontSizeLoaded) : 1.0f;
    
                     if (el.properties.align == "center") {
                         float textW = font->GetTextWidth(el.properties.text, scale, el.properties.fontName);
                         x += (el.geometry.w - textW) / 2;
                     } else if (el.properties.align == "right") {
                         float textW = font->GetTextWidth(el.properties.text, scale, el.properties.fontName);
                         x += (el.geometry.w - textW); 
                     } else { // padding
                         x += 10;
                     }
                     
                     glm::vec4 textColor = el.style.color;
                     if (textColor.a == 0.0f) textColor = glm::vec4(1.0f);
                     
                     font->RenderText(renderer, el.properties.text, x, y, scale, textColor, el.properties.fontName);
                 }
             }
        }
    }
}

glm::vec2 UIService::ScreenToUISpace(float screenX, float screenY, int windowW, int windowH)
{
    // Fixed Logical Resolution
    float logicalW = 1024.0f;
    float logicalH = 768.0f;
    
    // Simple scaling mapping
    float scaleX = logicalW / (float)windowW;
    float scaleY = logicalH / (float)windowH;
    
    return glm::vec2(screenX * scaleX, screenY * scaleY);
}

glm::vec2 UIService::NormalizeCoordinates(int screenX, int screenY)
{
    // Helper if we want to use current window size from SDL
    int w, h;
    SDL_Window* win = SDL_GL_GetCurrentWindow();
    SDL_GetWindowSize(win, &w, &h);
    return ScreenToUISpace((float)screenX, (float)screenY, w, h);
}

void UIService::HandleClick(float normalizedX, float normalizedY)
{
    if (m_activeScene.empty()) return;

    float mx = normalizedX;
    float my = normalizedY;
    
    // Check collision with buttons
    // Check collision
    auto& elements = m_layouts[m_activeScene];
    for (auto& el : elements)
    {
        bool hit = (mx >= el.geometry.x && mx <= el.geometry.x + el.geometry.w &&
                    my >= el.geometry.y && my <= el.geometry.y + el.geometry.h);

        if (hit) 
        {
            if (el.type == "BUTTON" && !el.properties.actionId.empty())
            {
                m_lastAction = el.properties.actionId;
                std::cout << "[DEBUG] UI Click Button: " << m_lastAction << std::endl;
                return; // Consume click
            }
            else if (el.type == "CHECKBOX")
            {
                el.properties.checked = !el.properties.checked;
                if (!el.properties.actionId.empty()) {
                     m_lastAction = el.properties.actionId; // Trigger action on toggle
                }
                std::cout << "[DEBUG] Checkbox Toggled: " << (el.properties.checked ? "ON" : "OFF") << std::endl;
                return;
            }
            else if (el.type == "SLIDER")
            {
                // Simple click-to-set logic for now (dragging requires state)
                float relX = mx - el.geometry.x;
                float pct = glm::clamp(relX / el.geometry.w, 0.0f, 1.0f);
                int range = el.properties.max - el.properties.min;
                el.properties.value = el.properties.min + (int)(pct * range);
                std::cout << "[DEBUG] Slider Value: " << el.properties.value << std::endl;
                return;
            }
        }
    }
}

std::string UIService::GetLastAction()
{
    return m_lastAction;
}

void UIService::ConsumeAction()
{
    m_lastAction = "";
}
