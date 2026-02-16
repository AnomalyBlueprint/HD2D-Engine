#pragma once
#include "Engine/Services/IService.h"
#include <GL/glew.h>
#include <vector>
#include <memory>
#include "Engine/Services/IShaderService.h"
#include <glm/glm.hpp>

/// <summary>
/// Manages usage of Framebuffer Objects (FBO) for post-processing effects.
/// Renders the scene to textures (Color, Normal, Depth) then applies outline shaders.
/// </summary>
class PostProcessService : public IService {
public:
    PostProcessService(int width, int height);
    ~PostProcessService();

    void Init() override;
    void Clean() override; 

    /// <summary>
    /// Binds the custom framebuffer. All subsequent rendering goes to texture.
    /// </summary>
    void Bind();

    /// <summary>
    /// Unbinds the framebuffer, returning rendering to the default window.
    /// </summary>
    void Unbind();
    
    /// <summary>
    /// Renders a full-screen quad applying the edge-detection shader.
    /// </summary>
    void RenderRect(std::shared_ptr<IShaderService> shaders, unsigned int shaderID, float normalThreshold, float depthThreshold, glm::vec4 outlineColor);

    unsigned int GetColorTexture() const { return m_colorTex; }
    unsigned int GetNormalTexture() const { return m_normalTex; }
    unsigned int GetDepthTexture() const { return m_depthTex; }

private:
    void SetupFBO(int width, int height);
    void SetupQuad();

    unsigned int m_fbo;
    unsigned int m_colorTex;
    unsigned int m_normalTex;
    unsigned int m_depthTex; 

    unsigned int m_quadVAO;
    unsigned int m_quadVBO;

    int m_screenWidth;
    int m_screenHeight;
};
