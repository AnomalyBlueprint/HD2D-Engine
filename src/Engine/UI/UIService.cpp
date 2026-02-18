#include "Engine/UI/UIService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/LoggerService.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IFontService.h"
#include "Engine/Services/TextureAtlasService.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm> 

glm::vec4 HexToVec4(const std::string& hex) {
    if (hex.empty() || hex == "transparent") return glm::vec4(0.0f);
    std::string h = hex;
    if (h[0] == '#') h = h.substr(1);
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
    glm::vec4 bg = HexToVec4(el->style.bg);
    if (bg.a > 0.0f) {
         Sprite s;
         s.Position = glm::vec2(el->geometry.x + el->geometry.w/2, el->geometry.y + el->geometry.h/2);
         s.Size = glm::vec2(el->geometry.w, el->geometry.h);
         s.Color = bg;
         if (el->textureId != 0) {
             s.TextureID = el->textureId;
             s.MinUV = el->uvMin; s.MaxUV = el->uvMax;
             s.Color = glm::vec4(1.0f); 
         }
         renderer->DrawSprite(s);
    }
    
    if (!el->text.empty()) {
         auto font = ServiceLocator::Get().GetService<IFontService>();
         if (font) {
              float scale = (el->fontSize > 0) ? el->fontSize/32.0f : 1.0f;
              glm::vec4 c = HexToVec4(el->style.color);
              font->RenderText(renderer, el->text, el->geometry.x, el->geometry.y + el->geometry.h/2 + 10, scale, c, el->fontName);
         }
    }
}

void UIService::HandleClick(float mx, float my) {
     for (auto& [guid, screen] : m_activeScreens) {
         auto root = screen->GetRoot();
         if (!root) continue;
         for (auto& el : root->children) {
             if (mx >= el->geometry.x && mx <= el->geometry.x + el->geometry.w &&
                 my >= el->geometry.y && my <= el->geometry.y + el->geometry.h) {
                 
                 if (!el->actionId.empty()) {
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
