#include "Engine/Services/PostProcessService.h"
#include <iostream>
#include <SDL.h>

PostProcessService::PostProcessService(int width, int height) 
    : m_fbo(0), m_colorTex(0), m_normalTex(0), m_depthTex(0), m_quadVAO(0), m_quadVBO(0), m_screenWidth(width), m_screenHeight(height) 
{
}

PostProcessService::~PostProcessService() {
    Clean();
}

void PostProcessService::OnInitialize() {
    SetupFBO(m_screenWidth, m_screenHeight);
    SetupQuad();
}

void PostProcessService::Clean() {
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_colorTex) glDeleteTextures(1, &m_colorTex);
    if (m_normalTex) glDeleteTextures(1, &m_normalTex);
    if (m_depthTex) glDeleteTextures(1, &m_depthTex);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
}

void PostProcessService::SetupFBO(int width, int height) {
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // 1. Color Attachment (RGB)
    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    // 2. Normal Attachment (RGB16F)
    glGenTextures(1, &m_normalTex);
    glBindTexture(GL_TEXTURE_2D, m_normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalTex, 0);

    // 3. Depth Attachment
    glGenTextures(1, &m_depthTex);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

    // Tell OpenGL which attachments we'll use (for MRT)
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

void PostProcessService::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_screenWidth, m_screenHeight);
}

void PostProcessService::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void PostProcessService::RenderRect(std::shared_ptr<IShaderService> shaders, unsigned int shaderID, float normalThreshold, float depthThreshold, glm::vec4 outlineColor)
{
    shaders->UseShader(shaderID);

    // TEXTURE UNIT 0: The Beauty Pass (Color)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    shaders->SetInt(shaderID, "u_color", 0);

    // TEXTURE UNIT 1: The Geometry Pass (Normals)
    // CRITICAL CHECK: Make sure this binds 'm_normalTex', NOT 'm_colorTex'
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalTex); 
    shaders->SetInt(shaderID, "u_normal", 1);

    // TEXTURE UNIT 2: The Depth Pass
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
    shaders->SetInt(shaderID, "u_depth", 2);

    // Set Thresholds (Tweak these to remove noise)
    // Higher normal threshold = ignores small bumps/noise
    shaders->SetFloat(shaderID, "u_normalThreshold", normalThreshold); 
    // Sensitive depth threshold for subtle edges
    shaders->SetFloat(shaderID, "u_depthThreshold", depthThreshold);
    // Outline Color
    shaders->SetVec4(shaderID, "u_outlineColor", outlineColor);

    // Draw
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
