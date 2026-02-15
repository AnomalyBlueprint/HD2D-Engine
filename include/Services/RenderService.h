#pragma once
#include "IService.h"
#include <SDL.h>
#include <vector>
#include "Core/Sprite.h"

class RenderService : public IService
{
public:
    virtual void Init() override = 0;
    virtual void Clean() override = 0;

    // Abstract Render Commands (The "Adapter" part)
    virtual void Clear() = 0;
    virtual void SwapBuffers() = 0;

    // UPDATED: Now takes 'indices' as well
    virtual unsigned int CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices) = 0;

    // UPDATED: Now takes 'indexCount' so we know how many points to draw
    virtual void DrawMesh(unsigned int meshID, int indexCount) = 0;
    
    // NEW: Load a texture and return its ID
    virtual unsigned int LoadTexture(const std::string &path) = 0;

    // NEW: Bind a texture to be used by the next Draw call
    virtual void UseTexture(unsigned int textureID) = 0;

    // Batch Rendering
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual void DrawSprite(const struct Sprite& sprite) = 0;
};