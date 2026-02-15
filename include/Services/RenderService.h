#pragma once
#include "IService.h"
#include <SDL.h>

class RenderService : public IService
{
public:
    virtual void Init() override = 0;
    virtual void Clean() override = 0;

    // Abstract Render Commands (The "Adapter" part)
    virtual void Clear() = 0;
    virtual void SwapBuffers() = 0;

    // Future: virtual void DrawMesh(Mesh* mesh) = 0;
};