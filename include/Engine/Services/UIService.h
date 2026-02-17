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
};

struct UIProperties {
    std::string text;
    int fontSize = 12;
    std::string align = "left";
    std::string actionId;
    bool checked = false;
    std::string label;
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

protected:
    void OnInitialize() override;

private:
    glm::vec4 HexToVec4(const std::string& hex);
    
    std::unordered_map<std::string, std::vector<UIElement>> m_layouts;
    std::string m_activeScene;
    int m_width = 1280;
    int m_height = 720;
};
