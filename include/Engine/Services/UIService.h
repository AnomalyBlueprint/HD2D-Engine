#pragma once
#include "Engine/Services/IUIService.h"
#include "Engine/Services/RenderService.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct UIGeometry {
    float x, y, w, h;
};

struct UIStyle {
    glm::vec4 bg = glm::vec4(0.0f);
    glm::vec4 border = glm::vec4(0.0f);
    glm::vec4 color = glm::vec4(1.0f);
    std::string image;
    unsigned int textureId = 0; // Custom OpenGL Texture ID
    glm::vec2 uvMin = glm::vec2(0.0f, 0.0f);
    glm::vec2 uvMax = glm::vec2(1.0f, 1.0f);
    float rotation = 0.0f;
};
// ... (skip down to class definition) ...


struct UIProperties {
    std::string text;
    int fontSize = 12;
    std::string fontName;
    std::string align = "left";
    std::string actionId;
    bool checked = false;
    std::string label;
    std::string prefabId; // New: If set, this element is a template
    std::string prefabHostId; // New: If set, this element hosts spawned prefabs
    float min = 0.0f;
    float max = 1.0f;
    int value = 50;
    std::string layoutMode = "free";
};

struct UIElement {
    std::string id;
    std::string type; // "CONTAINER", "BUTTON", "LABEL", "CHECKBOX"
    UIGeometry geometry;
    UIStyle style;
    UIProperties properties;
    std::vector<UIElement> children; // For nested structures if needed, though JSON seems flat-ish per scene
};

class UIService : public IUIService
{
public:
    UIService();
    virtual ~UIService();

    // IService
    void Clean() override;

    // IUIService
    void LoadLayouts(const std::string& path) override;
    void SetScene(const std::string& sceneName) override;
    void GetScreenSize(int& w, int& h) override;
    void Render(RenderService* renderer) override;
    
    // Input Handling
    // Input Handling
    void HandleClick(float normalizedX, float normalizedY) override;

    glm::vec2 NormalizeCoordinates(int screenX, int screenY) override;
    
    glm::vec2 ScreenToUISpace(float screenX, float screenY, int windowW, int windowH) override;

    std::string GetLastAction() override;
    void ConsumeAction() override;
    void SetElementText(const std::string& sceneName, const std::string& elementId, const std::string& text) override;
    void SetElementTexture(const std::string& sceneName, const std::string& elementId, unsigned int textureId) override;
    void SetElementUVs(const std::string& sceneName, const std::string& elementId, const glm::vec2& uvMin, const glm::vec2& uvMax) override;

protected:
    void OnInitialize() override;

private:
    glm::vec4 HexToVec4(const std::string& hex);
    
    std::unordered_map<std::string, std::vector<UIElement>> m_layouts;
    std::string m_activeScene;
    int m_width = 1280;
    int m_height = 720;
    
    std::string m_lastAction;

};
