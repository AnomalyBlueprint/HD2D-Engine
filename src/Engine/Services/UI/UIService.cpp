#include "Engine/Services/UI/UIService.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Services/Logging/LoggerService.h"
#include "Engine/Services/Rendering/RenderService.h"
#include "Engine/Services/Font/IFontService.h"
#include "Engine/Services/Rendering/TextureAtlasService.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm> 

glm::vec4 HexToVec4(const std::string& hex) {
    if (hex.empty() || hex == "transparent") return glm::vec4(0.0f);
    std::string h = hex;
    if (h[0] == '#') h = h.substr(1);
    
    // Support 3-digit shorthand hex (#ccc -> #cccccc, #fff -> #ffffff)
    if (h.length() == 3) {
        std::string expanded;
        expanded += h[0]; expanded += h[0];
        expanded += h[1]; expanded += h[1];
        expanded += h[2]; expanded += h[2];
        h = expanded;
    }
    // Support 4-digit shorthand hex (#rgba)
    if (h.length() == 4) {
        std::string expanded;
        expanded += h[0]; expanded += h[0];
        expanded += h[1]; expanded += h[1];
        expanded += h[2]; expanded += h[2];
        expanded += h[3]; expanded += h[3];
        h = expanded;
    }
    
    float r = 0, g = 0, b = 0, a = 1.0f;
    if (h.length() >= 6) {
        std::stringstream ss;
        ss << std::hex << h.substr(0, 2); int ri; ss >> ri; r = ri / 255.0f;
        ss.clear(); ss.str(""); ss << std::hex << h.substr(2, 2); int gi; ss >> gi; g = gi / 255.0f;
        ss.clear(); ss.str(""); ss << std::hex << h.substr(4, 2); int bi; ss >> bi; b = bi / 255.0f;
    }
    if (h.length() == 8) {
        std::stringstream ss;
        ss << std::hex << h.substr(6, 2); int ai; ss >> ai; a = ai / 255.0f;
    }
    return glm::vec4(r, g, b, a);
}

UIService::UIService() {}

UIService::~UIService() {}

void UIService::OnInitialize() {
    ServiceLocator::Get().GetService<ILoggerService>()->Log("UIService Initialized");
}

void UIService::Update() {}

void UIService::Clean() {
    m_activeScreens.clear();
    m_screenFactories.clear();
}

void UIService::LoadLayouts(const std::string& jsonPath) {
    try {
        std::ifstream f(jsonPath);
        if (!f.is_open()) return;
        f >> m_layouts;
        
        if (m_layouts.contains("settings")) {
            m_width = m_layouts["settings"].value("width", 1280);
            m_height = m_layouts["settings"].value("height", 720);
        }
    } catch (...) { /* Log error */ }
}

void UIService::RegisterScreenFactory(const std::string& screenId, std::function<std::shared_ptr<BaseScreen>(const std::string&)> factory) {
    m_screenFactories[screenId] = factory;
}

std::shared_ptr<UIElement> RecursiveParse(const nlohmann::json& node) {
    auto el = std::make_shared<UIElement>();
    if (node.contains("id")) el->id = node["id"];
    if (node.contains("type")) el->type = node["type"];
    
    if (node.contains("geometry")) {
        auto& g = node["geometry"];
        el->geometry = { g.value("x", 0.0f), g.value("y", 0.0f), g.value("w", 0.0f), g.value("h", 0.0f) };
    }
    
    if (node.contains("style")) {
        auto& s = node["style"];
        el->style.bg = s.value("bg", "");
        el->style.color = s.value("color", "");
        el->style.border = s.value("border", "");
         if (s.contains("image")) el->image = s["image"];
         if (s.contains("rotation")) el->rotation = s["rotation"];
         if (s.contains("textureId")) el->textureId = s["textureId"];
         if (s.contains("uvMin")) { el->uvMin = {s["uvMin"][0], s["uvMin"][1]}; }
         if (s.contains("uvMax")) { el->uvMax = {s["uvMax"][0], s["uvMax"][1]}; }
    }

    if (node.contains("properties")) {
        auto& p = node["properties"];
        if (p.contains("text")) el->text = p["text"];
        if (p.contains("actionId")) el->actionId = p["actionId"];
        if (p.contains("fontSize")) el->fontSize = p["fontSize"];
        if (p.contains("fontName")) el->fontName = p["fontName"];
        if (p.contains("align")) el->align = p["align"];
        if (p.contains("checked")) el->checked = p["checked"];
        if (p.contains("label")) el->label = p["label"];
        if (p.contains("min")) el->min = p["min"];
        if (p.contains("max")) el->max = p["max"];
        if (p.contains("value")) el->value = p["value"];
        if (p.contains("prefabId")) el->prefabId = p["prefabId"];
        if (p.contains("prefabHostId")) el->prefabHostId = p["prefabHostId"];
        if (p.contains("layoutMode")) el->layoutMode = p["layoutMode"];
    }
    return el;
}

std::shared_ptr<UIElement> UIService::ParseLayout(const std::string& screenId) {
    if (!m_layouts.contains("scenes")) return nullptr;
    if (!m_layouts["scenes"].contains(screenId)) return nullptr;

    auto root = std::make_shared<UIElement>();
    root->id = screenId;
    root->type = "SCREEN_ROOT";
    
    auto& sceneJson = m_layouts["scenes"][screenId];
    if (sceneJson.is_array()) {
        for (auto& item : sceneJson) {
            root->children.push_back(RecursiveParse(item));
        }
    }
    return root;
}

std::string UIService::OpenScreen(const std::string& screenId) {
    std::string guid = GenerateGuid();
    std::shared_ptr<BaseScreen> screen;

    if (m_screenFactories.find(screenId) != m_screenFactories.end()) {
         screen = m_screenFactories[screenId](guid);
    } else {
         screen = std::make_shared<BaseScreen>(screenId, guid); 
    }
    
    auto layout = ParseLayout(screenId);
    if (layout) {
        screen->OnLoad(layout);
    }
    
    screen->OnShow();
    m_activeScreens[guid] = screen;
    return guid;
}

void UIService::CloseScreen(const std::string& guid) {
    if (m_activeScreens.count(guid)) {
        m_activeScreens[guid]->OnHide();
        m_activeScreens[guid]->OnUnload();
        m_activeScreens.erase(guid);
    }
}

void UIService::SetScene(const std::string& sceneName) {
    m_activeScreens.clear(); 
    OpenScreen(sceneName);
    m_activeSceneId = sceneName; 
}

void UIService::Render(RenderService* renderer) {
     if (!renderer) return;
     for (auto& [guid, screen] : m_activeScreens) {
         auto root = screen->GetRoot();
         if (!root) continue;
         for (auto& el : root->children) {
             RenderElement(renderer, el);
         }
     }

     if (m_activePopup.has_value()) {
         // Simple overlay for popup
         Sprite overlay;
         overlay.Position = glm::vec2(m_width/2, m_height/2);
         overlay.Size = glm::vec2(m_width, m_height);
         overlay.Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.7f); // Dim background
         renderer->DrawSprite(overlay);
         
         // TODO: Render actual popup box and text using a standard prefab or hardcoded box
         // For now, the Log output confirms it is "Active"
     }
}

void UIService::RenderElement(RenderService* renderer, std::shared_ptr<UIElement> el) {
    // Skip Prefab Templates (they are cloned, not rendered directly)
    if (!el->prefabId.empty()) return;

    glm::vec4 bg = HexToVec4(el->style.bg);
    glm::vec4 borderColor = HexToVec4(el->style.border);
    auto atlas = ServiceLocator::Get().GetService<TextureAtlasService>();
    
    // --- 1. Draw Background / Image / Texture ---
    if (!el->image.empty() && atlas) {
         // Image from texture atlas
         Sprite imgSprite;
         imgSprite.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h/2.0f);
         imgSprite.Size = glm::vec2(el->geometry.w, el->geometry.h);
         imgSprite.Color = glm::vec4(1.0f); // White tint for texture
         imgSprite.TextureID = atlas->GetTextureID();
         imgSprite.Rotation = el->rotation;
         
         UVRect uvs = atlas->GetUVs(el->image);
         imgSprite.MinUV = glm::vec2(uvs.uMin, uvs.vMin);
         imgSprite.MaxUV = glm::vec2(uvs.uMax, uvs.vMax);
         
         renderer->DrawSprite(imgSprite);
    } else if (bg.a > 0.0f) {
         // Solid background color
         Sprite bgSprite;
         bgSprite.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h/2.0f);
         bgSprite.Size = glm::vec2(el->geometry.w, el->geometry.h);
         bgSprite.Color = bg;
         bgSprite.TextureID = 0;
         renderer->DrawSprite(bgSprite);
    } else if (el->textureId != 0) {
         // Dynamic texture (e.g. map visualizer)
         Sprite texSprite;
         texSprite.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h/2.0f);
         texSprite.Size = glm::vec2(el->geometry.w, el->geometry.h);
         texSprite.Color = glm::vec4(1.0f);
         texSprite.TextureID = el->textureId;
         texSprite.MinUV = el->uvMin;
         texSprite.MaxUV = el->uvMax;
         texSprite.Rotation = el->rotation;
         renderer->DrawSprite(texSprite);
    }

    // --- Border Rendering ---
    if (borderColor.a > 0.0f) {
         float thickness = 2.0f;
         
         Sprite top;
         top.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + thickness/2.0f);
         top.Size = glm::vec2(el->geometry.w, thickness);
         top.Color = borderColor;
         top.TextureID = 0;
         renderer->DrawSprite(top);

         Sprite bot;
         bot.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h - thickness/2.0f);
         bot.Size = glm::vec2(el->geometry.w, thickness);
         bot.Color = borderColor;
         bot.TextureID = 0;
         renderer->DrawSprite(bot);

         Sprite left;
         left.Position = glm::vec2(el->geometry.x + thickness/2.0f, el->geometry.y + el->geometry.h/2.0f);
         left.Size = glm::vec2(thickness, el->geometry.h);
         left.Color = borderColor;
         left.TextureID = 0;
         renderer->DrawSprite(left);

         Sprite right;
         right.Position = glm::vec2(el->geometry.x + el->geometry.w - thickness/2.0f, el->geometry.y + el->geometry.h/2.0f);
         right.Size = glm::vec2(thickness, el->geometry.h);
         right.Color = borderColor;
         right.TextureID = 0;
         renderer->DrawSprite(right);
    }

    // --- 2. Draw Widget-Specific Visuals ---
    if (el->type == "SLIDER") {
         // Draw Track
         Sprite trackSprite;
         trackSprite.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h/2.0f);
         trackSprite.Size = glm::vec2(el->geometry.w, el->geometry.h/4.0f);
         trackSprite.Color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
         trackSprite.TextureID = 0;
         renderer->DrawSprite(trackSprite);

         // Draw Knob
         float range = (float)(el->max - el->min);
         float pct = (range > 0) ? ((float)(el->value - el->min) / range) : 0.0f;
         float knobX = el->geometry.x + (el->geometry.w * pct);
         
         Sprite knobSprite;
         knobSprite.Position = glm::vec2(knobX, el->geometry.y + el->geometry.h/2.0f);
         knobSprite.Size = glm::vec2(10.0f, el->geometry.h);
         knobSprite.Color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
         knobSprite.TextureID = 0;
         renderer->DrawSprite(knobSprite);
    } else if (el->type == "CHECKBOX") {
         // Draw Box
         Sprite boxSprite;
         boxSprite.Position = glm::vec2(el->geometry.x + el->geometry.w/2.0f, el->geometry.y + el->geometry.h/2.0f);
         boxSprite.Size = glm::vec2(el->geometry.w, el->geometry.h);
         boxSprite.Color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
         boxSprite.TextureID = 0;
         renderer->DrawSprite(boxSprite);

         if (el->checked) {
             Sprite checkSprite;
             checkSprite.Position = boxSprite.Position;
             checkSprite.Size = boxSprite.Size * 0.6f;
             checkSprite.Color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
             checkSprite.TextureID = 0;
             renderer->DrawSprite(checkSprite);
         }
         
         // Draw Label if exists
         if (!el->label.empty()) {
             auto font = ServiceLocator::Get().GetService<IFontService>();
             if (font) {
                 font->RenderText(renderer, el->label, 
                     el->geometry.x + el->geometry.w + 10, 
                     el->geometry.y + el->geometry.h/2 + 5, 
                     0.5f, glm::vec4(1.0f), el->fontName);
             }
         }
    }
    
    // --- 3. Draw Text / Content ---
    if (el->type == "LABEL" || el->type == "BUTTON") {
         if (!el->text.empty()) {
             auto font = ServiceLocator::Get().GetService<IFontService>();
             if (font) {
                 float x = el->geometry.x;
                 float y = el->geometry.y + el->geometry.h/2 + 10;
                 
                 float fontSizeLoaded = 32.0f;
                 float scale = (el->fontSize > 0) ? (el->fontSize / fontSizeLoaded) : 1.0f;

                 if (el->align == "center") {
                     float textW = font->GetTextWidth(el->text, scale, el->fontName);
                     x += (el->geometry.w - textW) / 2;
                 } else if (el->align == "right") {
                     float textW = font->GetTextWidth(el->text, scale, el->fontName);
                     x += (el->geometry.w - textW);
                 } else {
                     x += 10; // Left padding
                 }
                 
                 glm::vec4 textColor = HexToVec4(el->style.color);
                 if (textColor.a == 0.0f) textColor = glm::vec4(1.0f);
                 
                 font->RenderText(renderer, el->text, x, y, scale, textColor, el->fontName);
             }
         }
    }
}

void UIService::HandleClick(float mx, float my) {
     for (auto& [guid, screen] : m_activeScreens) {
         auto root = screen->GetRoot();
         if (!root) continue;
         for (auto& el : root->children) {
             bool hit = (mx >= el->geometry.x && mx <= el->geometry.x + el->geometry.w &&
                         my >= el->geometry.y && my <= el->geometry.y + el->geometry.h);
             
             if (hit) {
                 if (el->type == "BUTTON" && !el->actionId.empty()) {
                     m_lastAction = el->actionId;
                     return; // Consume click
                 } else if (el->type == "CHECKBOX") {
                     el->checked = !el->checked;
                     if (!el->actionId.empty()) {
                         m_lastAction = el->actionId;
                     }
                     return;
                 } else if (el->type == "SLIDER") {
                     float relX = mx - el->geometry.x;
                     float pct = glm::clamp(relX / el->geometry.w, 0.0f, 1.0f);
                     int range = el->max - el->min;
                     el->value = el->min + (int)(pct * range);
                     return;
                 } else if (!el->actionId.empty()) {
                     m_lastAction = el->actionId;
                 }
             }
         }
     }
}

std::string UIService::GetLastAction() { return m_lastAction; }
void UIService::ConsumeAction() { m_lastAction = ""; }

glm::vec2 UIService::ScreenToUISpace(float sx, float sy, int ww, int wh) {
     return glm::vec2(sx * (1024.0f/ww), sy * (768.0f/wh));
}

void UIService::GetScreenSize(int& w, int& h) { w = m_width; h = m_height; }

void UIService::SetElementText(const std::string& sceneName, const std::string& elementId, const std::string& text) {
     for (auto& [guid, screen] : m_activeScreens) {
         if (screen->GetId() == sceneName) {
             auto el = screen->FindElement(elementId);
             if (el) el->text = text;
         }
     }
}

void UIService::SetElementTexture(const std::string& sceneName, const std::string& elementId, unsigned int textureId) {
     for (auto& [guid, screen] : m_activeScreens) {
         if (screen->GetId() == sceneName) {
             auto el = screen->FindElement(elementId);
             if (el) el->textureId = textureId;
         }
     }
}

void UIService::SetElementUVs(const std::string& sceneName, const std::string& elementId, const glm::vec2& min, const glm::vec2& max) {
     for (auto& [guid, screen] : m_activeScreens) {
         if (screen->GetId() == sceneName) {
             auto el = screen->FindElement(elementId);
             if (el) { el->uvMin = min; el->uvMax = max; }
         }
     }
}

void UIService::ShowPopup(const PopupRequest& req) { 
    m_popupQueue.push(req);
    ProcessPopupQueue();
}

void UIService::CloseActivePopup() { 
    m_activePopup.reset(); 
    ProcessPopupQueue();
}

void UIService::ProcessPopupQueue() {
    if (m_activePopup.has_value()) return; // Already showing one
    if (m_popupQueue.empty()) return; // Nothing to show

    m_activePopup = m_popupQueue.top();
    m_popupQueue.pop();
    
    // Log for verification
    ServiceLocator::Get().GetService<ILoggerService>()->Log("Showing Popup: " + m_activePopup->title);
}

std::shared_ptr<UIElement> UIService::DeepClone(std::shared_ptr<UIElement> source) {
    if (!source) return nullptr;
    auto clone = std::make_shared<UIElement>(*source); // Component-wise copy
    clone->children.clear(); // Clear shared pointer refs
    for (auto& child : source->children) {
        clone->children.push_back(DeepClone(child));
    }
    return clone;
}

std::shared_ptr<UIElement> UIService::ParsePrefab(const std::string& prefabId) {
    if (!m_layouts.contains("prefabs")) return nullptr;
    if (!m_layouts["prefabs"].contains(prefabId)) return nullptr;

    auto& prefabJson = m_layouts["prefabs"][prefabId];
    return RecursiveParse(prefabJson);
}

std::shared_ptr<UIElement> UIService::InstantiatePrefab(const std::string& prefabId, std::shared_ptr<UIElement> parent) {
    auto prefab = ParsePrefab(prefabId);
    if (!prefab) {
        ServiceLocator::Get().GetService<ILoggerService>()->Log("Error: Prefab not found: " + prefabId);
        return nullptr;
    }
    
    // Clone it so we have a unique instance
    auto instance = DeepClone(prefab);
    
    if (parent) {
        parent->children.push_back(instance);
    }
    
    return instance;
}

std::string UIService::GenerateGuid() { 
    static int id = 0; return std::to_string(++id); 
}
