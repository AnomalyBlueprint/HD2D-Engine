#include "Engine/Services/PostProcessService.h"
#include <iostream>
#include <SDL.h>

PostProcessService::PostProcessService(int width, int height) 
    : m_screenWidth(width), m_screenHeight(height), m_quadVAO(0), m_quadVBO(0) 
{
}

PostProcessService::~PostProcessService() {
    Clean();
}

void PostProcessService::OnInitialize() {
    CreateLayer("World", true); // Color, Normal, Depth
    CreateLayer("UI", false);   // Color only (RGBA)
    SetupQuad();
}

void PostProcessService::Clean() {
    for (auto& [name, layer] : m_layers) {
        if (layer.fbo) glDeleteFramebuffers(1, &layer.fbo);
        if (layer.colorTex) glDeleteTextures(1, &layer.colorTex);
        if (layer.normalTex) glDeleteTextures(1, &layer.normalTex);
        if (layer.depthTex) glDeleteTextures(1, &layer.depthTex);
    }
    m_layers.clear();

    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
}

void PostProcessService::CreateLayer(const std::string& name, bool useDepthNormal) {
    if (m_layers.find(name) != m_layers.end()) return;

    PostProcessLayer layer;
    glGenFramebuffers(1, &layer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, layer.fbo);

    // 1. Color Attachment
    glGenTextures(1, &layer.colorTex);
    glBindTexture(GL_TEXTURE_2D, layer.colorTex);
    
    // UI needs alpha for blending (RGBA), World uses RGB (or RGBA)
    GLint internalFormat = useDepthNormal ? GL_RGB : GL_RGBA;
    GLenum format = useDepthNormal ? GL_RGB : GL_RGBA;
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_screenWidth, m_screenHeight, 0, format, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layer.colorTex, 0);

    if (useDepthNormal) {
        // 2. Normal Attachment (RGB16F)
        glGenTextures(1, &layer.normalTex);
        glBindTexture(GL_TEXTURE_2D, layer.normalTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_screenWidth, m_screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, layer.normalTex, 0);

        // 3. Depth Attachment
        glGenTextures(1, &layer.depthTex);
        glBindTexture(GL_TEXTURE_2D, layer.depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_screenWidth, m_screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, layer.depthTex, 0);

        // Tell OpenGL to draw to Color and Normal
        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    } else {
        // Simple Layer (UI) - Only Color
        unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Layer " << name << " is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_layers[name] = layer;
}

void PostProcessService::SetupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void PostProcessService::SetTargetLayer(const std::string& layerName) {
    auto it = m_layers.find(layerName);
    if (it != m_layers.end()) {
        glBindFramebuffer(GL_FRAMEBUFFER, it->second.fbo);
        glViewport(0, 0, m_screenWidth, m_screenHeight);
        m_currentLayer = layerName;
    }
}

void PostProcessService::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void PostProcessService::RenderLayerWithEdges(const std::string& layerName, std::shared_ptr<IShaderService> shaders, unsigned int shaderID, float normalThreshold, float depthThreshold, glm::vec4 outlineColor)
{
    auto it = m_layers.find(layerName);
    if (it == m_layers.end()) return;
    
    const PostProcessLayer& layer = it->second;

    shaders->UseShader(shaderID);

    // TEXTURE UNIT 0: The Beauty Pass (Color)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, layer.colorTex);
    shaders->SetInt(shaderID, "u_color", 0);

    // TEXTURE UNIT 1: The Geometry Pass (Normals)
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, layer.normalTex); 
    shaders->SetInt(shaderID, "u_normal", 1);

    // TEXTURE UNIT 2: The Depth Pass
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, layer.depthTex);
    shaders->SetInt(shaderID, "u_depth", 2);

    // Set Thresholds
    shaders->SetFloat(shaderID, "u_normalThreshold", normalThreshold); 
    shaders->SetFloat(shaderID, "u_depthThreshold", depthThreshold);
    shaders->SetVec4(shaderID, "u_outlineColor", outlineColor);

    // Draw
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessService::RenderLayerComposite(const std::string& layerName, std::shared_ptr<IShaderService> shaders, unsigned int shaderID)
{
    auto it = m_layers.find(layerName);
    if (it == m_layers.end()) return;
    
    if (shaderID > 0) shaders->UseShader(shaderID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, it->second.colorTex);
    if (shaderID > 0) shaders->SetInt(shaderID, "u_texture", 0); 

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int PostProcessService::GetTexture(const std::string& layerName) {
    if (m_layers.find(layerName) != m_layers.end()) return m_layers[layerName].colorTex;
    return 0;
}
