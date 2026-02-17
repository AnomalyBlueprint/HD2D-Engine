#pragma once
#include <glm/glm.hpp>
#include <SDL.h>

/// <summary>
/// Represents a 2D sprite to be rendered in the world.
/// </summary>
struct Sprite
{
    unsigned int TextureID = 0;             ///< ID of the texture to use.
    glm::vec2 Position = {0.0f, 0.0f};      ///< World position of the sprite.
    glm::vec2 Size = {100.0f, 100.0f};      ///< Size of the sprite in pixels.
    float Rotation = 0.0f;                  ///< Rotation in degrees.
    glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f}; ///< Tint color (RGBA).
    SDL_Rect SourceRect = {0, 0, 0, 0};     ///< Source rectangle for spritesheets (0,0,0,0 = full texture).
    
    // UV Coordinates (0.0 to 1.0)
    glm::vec2 MinUV = {0.0f, 0.0f};
    glm::vec2 MaxUV = {1.0f, 1.0f};
};
