#pragma once
#include "Engine/Services/IService.h"
#include "Engine/Services/IInputService.h"
#include <string>

class RenderService; // Forward declaration

/// <summary>
/// Interface for the UI Service, handling 2D layout loading and rendering.
/// </summary>
class IUIService : public IService
{
public:
    virtual ~IUIService() = default;

    /// <summary>
    /// Loads UI layouts from a JSON file.
    /// </summary>
    /// <param name="path">Path to the JSON file.</param>
    virtual void LoadLayouts(const std::string& path) = 0;

    /// <summary>
    /// Sets the active UI scene to render.
    /// </summary>
    /// <param name="sceneName">Name of the scene (e.g., "main_menu").</param>
    virtual void SetScene(const std::string& sceneName) = 0;

    /// <summary>
    /// Gets the screen size defined in the UI layout.
    /// </summary>
    /// <param name="w">Output width.</param>
    /// <param name="h">Output height.</param>
    virtual void GetScreenSize(int& w, int& h) = 0;

    /// <summary>
    /// Renders the active UI scene using the provided renderer.
    /// </summary>
    /// <param name="renderer">Pointer to the RenderService.</param>
    /// <summary>
    /// Renders the active UI scene using the provided renderer.
    /// </summary>
    /// <param name="renderer">Pointer to the RenderService.</param>
    virtual void Render(RenderService* renderer) = 0;

    // Input Handling
    virtual void Update(IInputService* input) = 0;
    virtual std::string GetLastAction() = 0;
    virtual void ConsumeAction() = 0;
};
