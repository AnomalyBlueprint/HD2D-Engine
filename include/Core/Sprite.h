#pragma once
#include <glm/glm.hpp>
#include <SDL.h>

struct Sprite
{
    unsigned int TextureID = 0;
    glm::vec2 Position = {0.0f, 0.0f};
    glm::vec2 Size = {100.0f, 100.0f};
    float Rotation = 0.0f; // Degrees
    glm::vec4 Color = {1.0f, 1.0f, 1.0f, 1.0f}; // Default White
    SDL_Rect SourceRect = {0, 0, 0, 0}; // 0,0,0,0 means Draw Full Texture
};
