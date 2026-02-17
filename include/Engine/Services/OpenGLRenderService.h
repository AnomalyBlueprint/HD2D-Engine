#pragma once
#include "Engine/Services/RenderService.h"
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Engine/Core/Vertex.h"

/// <summary>
/// OpenGL implementation of the Render Service.
/// Handles SDL Window Context, GLEW initialization, and Batch Rendering.
/// </summary>
class OpenGLRenderService : public RenderService
{
public:
    OpenGLRenderService(SDL_Window *window);
    ~OpenGLRenderService();

protected:
    void OnInitialize() override;
public:
    void Clean() override;
    void Clear() override;
    void SwapBuffers() override;
    void SetDepthTest(bool enable) override;
    void SetViewport(int width, int height) override;
    unsigned int CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices) override;
    void DrawMesh(unsigned int meshID, int indexCount) override;
    unsigned int LoadTexture(const std::string &path) override;
    void UseTexture(unsigned int textureID) override;

    void Begin(const glm::mat4& projectionMatrix = glm::mat4(1.0f)) override;
    void End() override;
    void DrawSprite(const Sprite& sprite) override;
    void SetClearColor(float r, float g, float b, float a) override;

private:
    void Flush();

    SDL_Window *window;
    SDL_GLContext context;

    // Batch Data
    // Vertex struct is now in Core/Vertex.h
    
    const size_t MAX_SPRITES = 10000;
    const size_t MAX_VERTICES = MAX_SPRITES * 4;
    const size_t MAX_INDICES = MAX_SPRITES * 6;

    unsigned int m_batchVAO = 0;
    unsigned int m_batchVBO = 0;
    unsigned int m_batchEBO = 0;

    std::vector<Vertex> m_vertices;
    unsigned int m_currentTextureID = 0;
    unsigned int m_whiteTextureID = 0; // Default White Texture

    glm::vec4 m_clearColor{0.39f, 0.58f, 0.93f, 1.0f}; // Default Blue
    glm::mat4 m_projectionMatrix{1.0f};
};