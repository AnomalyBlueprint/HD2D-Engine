#pragma once
#include "Services/IService.h"
#include <GL/glew.h>
#include <vector>
#include <memory>
#include "Services/IShaderService.h"
#include <glm/glm.hpp>

class PostProcessService : public IService {
public:
    PostProcessService(int width, int height);
    ~PostProcessService();

    void Init() override; // Restore IService override
    void Clean() override; 

    // Framebuffer Operations
    void Bind();
    void Unbind();
    
    // Rendering
    void RenderRect(std::shared_ptr<IShaderService> shaders, unsigned int shaderID, float normalThreshold, float depthThreshold, glm::vec4 outlineColor);

    // Getters for textures
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
