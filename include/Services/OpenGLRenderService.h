#pragma once
#include "Services/RenderService.h"
#include <SDL.h>
#include <GL/glew.h>

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

private:
    SDL_Window *window;
    SDL_GLContext context;
};