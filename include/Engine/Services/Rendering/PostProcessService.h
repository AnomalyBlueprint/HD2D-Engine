#pragma once
#include "Engine/Core/IService.h"
#include <GL/glew.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "Engine/Services/Rendering/IShaderService.h"
#include <glm/glm.hpp>

struct PostProcessLayer {
    unsigned int fbo = 0;
    unsigned int colorTex = 0;
    unsigned int normalTex = 0;
    unsigned int depthTex = 0;
};

/// <summary>
/// Manages usage of Framebuffer Objects (FBO) for post-processing effects.
/// Renders the scene to textures (Color, Normal, Depth) then applies outline shaders.
/// </summary>
class PostProcessService : public IService {
public:
    PostProcessService(int width, int height);
    ~PostProcessService();

protected:
    void OnInitialize() override;
public:
    void Clean() override; 

    /// <summary>
    /// Binds the custom framebuffer of the specified layer.
    /// </summary>
    void SetTargetLayer(const std::string& layerName);

    /// <summary>
    /// Unbinds the framebuffer, returning rendering to the default window.
    /// </summary>
    void Unbind();
    
    /// <summary>
    /// Renders a layer using the Edge Detection shader.
    /// </summary>
    void RenderLayerWithEdges(const std::string& layerName, std::shared_ptr<IShaderService> shaders, unsigned int shaderID, float normalThreshold, float depthThreshold, glm::vec4 outlineColor);

    /// <summary>
    /// Renders a layer as a simple textured quad (for UI overlay).
    /// </summary>
    void RenderLayerComposite(const std::string& layerName, std::shared_ptr<IShaderService> shaders, unsigned int shaderID);

    unsigned int GetTexture(const std::string& layerName);

private:
    void CreateLayer(const std::string& name, bool useDepthNormal);
    void SetupQuad();

    std::map<std::string, PostProcessLayer> m_layers;
    std::string m_currentLayer;

    unsigned int m_quadVAO;
    unsigned int m_quadVBO;

    int m_screenWidth;
    int m_screenHeight;
};
