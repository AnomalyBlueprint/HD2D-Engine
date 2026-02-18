#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

struct Rect {
    float x, y, w, h;
};

struct UIStyle {
    std::string bg;
    std::string color;
    std::string border;
};

class UIElement {
public:
    std::string id;
    std::string type;
    Rect geometry;
    UIStyle style;
    std::vector<std::shared_ptr<UIElement>> children;
    
    // Properties
    std::string text;
    std::string actionId;
    std::string image;
    
    // Rendering props
    float fontSize = 12.0f;
    std::string fontName;
    std::string align = "left";
    float rotation = 0.0f;
    unsigned int textureId = 0;
    glm::vec2 uvMin = {0,0};
    glm::vec2 uvMax = {1,1};
    
    // Widget-specific properties
    bool checked = false;
    std::string label;          // Checkbox label text
    int min = 0;                // Slider min
    int max = 100;              // Slider max
    int value = 50;             // Slider current value
    std::string prefabId;       // Prefab template ID
    std::string prefabHostId;   // Prefab host container ID
    std::string layoutMode = "free";
    
    UIElement() : geometry{0,0,0,0} {}
    
    std::shared_ptr<UIElement> FindChild(const std::string& searchId) {
        if (id == searchId) return nullptr; 
        for (auto& child : children) {
            if (child->id == searchId) return child;
            auto found = child->FindChild(searchId);
            if (found) return found;
        }
        return nullptr;
    }
};
