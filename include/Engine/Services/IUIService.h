#pragma once
#include "Engine/Services/IService.h"
#include "Engine/UI/BaseScreen.h"
#include <string>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

class RenderService; // Forward decl

enum class UILayer {
    Background = 0,
    Screen = 1,
    Overlay = 2,
    Modal = 3
};

enum class PopupPriority {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3
};

struct PopupRequest {
    std::string id;
    PopupPriority priority;
    std::string title;
    std::string message;
    std::function<void()> onConfirm;
    std::function<void()> onCancel;
};

class IUIService : public IService
{
public:
    virtual ~IUIService() = default;

    // JSON Loading
    virtual void LoadLayouts(const std::string& path) = 0;

    // Registry
    virtual void RegisterScreenFactory(const std::string& screenId, std::function<std::shared_ptr<BaseScreen>(const std::string&)> factory) = 0;

    // Lifecycle
    virtual std::string OpenScreen(const std::string& screenId) = 0;
    virtual void CloseScreen(const std::string& guid) = 0;
    virtual std::shared_ptr<UIElement> InstantiatePrefab(const std::string& prefabId, std::shared_ptr<UIElement> parent = nullptr) = 0;

    // Popups
    virtual void ShowPopup(const PopupRequest& request) = 0;
    virtual void CloseActivePopup() = 0;
    
    // Rendering & Input (Compatibility Layer)
    virtual void Render(RenderService* renderer) = 0;
    virtual void HandleClick(float normalizedX, float normalizedY) = 0;
    virtual void GetScreenSize(int& w, int& h) = 0;
    virtual glm::vec2 ScreenToUISpace(float screenX, float screenY, int windowW, int windowH) = 0;
    
    // Legacy/Helpers used by MainMenuScene
    virtual std::string GetLastAction() = 0;
    virtual void ConsumeAction() = 0;
    virtual void SetScene(const std::string& sceneName) = 0; // Deprecate or alias to OpenScreen
    
    // Dynamic Updates (used by Macro Gen)
    virtual void SetElementText(const std::string& sceneName, const std::string& elementId, const std::string& text) = 0;
    virtual void SetElementTexture(const std::string& sceneName, const std::string& elementId, unsigned int textureId) = 0;
    virtual void SetElementUVs(const std::string& sceneName, const std::string& elementId, const glm::vec2& uvMin, const glm::vec2& uvMax) = 0;
};
