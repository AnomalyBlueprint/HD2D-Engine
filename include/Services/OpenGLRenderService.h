#pragma once
#include "Services/RenderService.h"
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Core/Vertex.h"

class OpenGLRenderService : public RenderService
{
public:
    OpenGLRenderService(SDL_Window *window);
    ~OpenGLRenderService();

    void Init() override;
    void Clean() override;
    void Clear() override;
    void SwapBuffers() override;
    unsigned int CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices) override;
    void DrawMesh(unsigned int meshID, int indexCount) override;
    unsigned int LoadTexture(const std::string &path) override;
    void UseTexture(unsigned int textureID) override;

    // Batch Rendering
    void Begin() override;
    void End() override;
    void DrawSprite(const Sprite& sprite) override;

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
};