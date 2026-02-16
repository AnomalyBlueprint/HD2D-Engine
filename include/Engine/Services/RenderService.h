#pragma once
#include "IService.h"
#include <SDL.h>
#include <vector>
#include "Engine/Core/Sprite.h"

/// <summary>
/// Abstract base class for the Rendering System.
/// Defines the contract for drawing meshes, sprites, and handling textures.
/// </summary>
class RenderService : public IService
{
public:
    virtual void Init() override = 0;
    virtual void Clean() override = 0;

    // --- Core Rendering ---

    /// <summary>
    /// Clears the screen buffers (Color & Depth).
    /// </summary>
    virtual void Clear() = 0;

    /// <summary>
    /// Swaps the front and back buffers to display the frame.
    /// </summary>
    virtual void SwapBuffers() = 0;

    // --- Mesh Rendering ---

    /// <summary>
    /// Uploads mesh data to the GPU and returns a handle (Mesh ID / VAO).
    /// </summary>
    virtual unsigned int CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices) = 0;

    /// <summary>
    /// Draws a previously created mesh.
    /// </summary>
    virtual void DrawMesh(unsigned int meshID, int indexCount) = 0;
    
    // --- Texture Management ---

    /// <summary>
    /// Loads a texture from disk and returns its GPU ID.
    /// </summary>
    virtual unsigned int LoadTexture(const std::string &path) = 0;

    /// <summary>
    /// Binds a specific texture for subsequent draw calls.
    /// </summary>
    virtual void UseTexture(unsigned int textureID) = 0;

    // --- Batch Rendering (2D Sprites / UI) ---

    /// <summary>
    /// Begins a batch rendering frame. Resets buffers.
    /// </summary>
    virtual void Begin() = 0;

    /// <summary>
    /// Ends the batch rendering frame and flushes remaining geometry.
    /// </summary>
    virtual void End() = 0;

    /// <summary>
    /// Adds a sprite to the current batch.
    /// </summary>
    virtual void DrawSprite(const struct Sprite& sprite) = 0;
};