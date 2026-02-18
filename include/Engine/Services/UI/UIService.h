#pragma once

#include "Engine/Services/IUIService.h"
#include "Engine/UI/BaseScreen.h"
#include "nlohmann/json.hpp" 
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <queue>
#include <optional>

class UIService : public IUIService {
public:
    UIService();
    ~UIService();

    void OnInitialize() override;
    void Update() override; 
    void Clean() override;

    void LoadLayouts(const std::string& jsonPath) override;
    void RegisterScreenFactory(const std::string& screenId, std::function<std::shared_ptr<BaseScreen>(const std::string&)> factory) override;
    std::string OpenScreen(const std::string& screenId) override;
    void CloseScreen(const std::string& guid) override;
    std::shared_ptr<UIElement> InstantiatePrefab(const std::string& prefabId, std::shared_ptr<UIElement> parent = nullptr) override;
    void ShowPopup(const PopupRequest& request) override;
    void CloseActivePopup() override;

    // Compatibility
    void Render(RenderService* renderer) override;
    void HandleClick(float normalizedX, float normalizedY) override;
    void SetScene(const std::string& sceneName) override;
    std::string GetLastAction() override;
    void ConsumeAction() override;
    glm::vec2 ScreenToUISpace(float screenX, float screenY, int windowW, int windowH) override;
    void GetScreenSize(int& w, int& h) override;

    void SetElementText(const std::string& sceneName, const std::string& elementId, const std::string& text) override;
    void SetElementTexture(const std::string& sceneName, const std::string& elementId, unsigned int textureId) override;
    void SetElementUVs(const std::string& sceneName, const std::string& elementId, const glm::vec2& uvMin, const glm::vec2& uvMax) override;

private:
    void RenderElement(RenderService* renderer, std::shared_ptr<UIElement> el);
    
    struct PopupComparator {
        bool operator()(const PopupRequest& a, const PopupRequest& b) {
            return static_cast<int>(a.priority) < static_cast<int>(b.priority);
        }
    };

    std::string GenerateGuid();
    std::shared_ptr<UIElement> ParseLayout(const std::string& screenId);
    std::shared_ptr<UIElement> ParsePrefab(const std::string& prefabId);
    void ProcessPopupQueue();
    std::shared_ptr<UIElement> DeepClone(std::shared_ptr<UIElement> source);
    
    nlohmann::json m_layouts; 
    
    std::map<std::string, std::function<std::shared_ptr<BaseScreen>(const std::string&)>> m_screenFactories;
    std::map<std::string, std::shared_ptr<BaseScreen>> m_activeScreens;

    std::priority_queue<PopupRequest, std::vector<PopupRequest>, PopupComparator> m_popupQueue;
    std::optional<PopupRequest> m_activePopup;
    
    int m_width = 1280;
    int m_height = 720;
    std::string m_lastAction;
    std::string m_activeSceneId;
};
