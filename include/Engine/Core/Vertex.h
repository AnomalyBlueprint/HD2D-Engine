#pragma once
#include <glm/glm.hpp>

/// <summary>
/// Represents a single vertex in the geometry pipeline.
/// </summary>
struct Vertex {
    glm::vec3 position; ///< Position in 3D space.
    glm::vec4 color;    ///< RGBA color.
    glm::vec2 texCoord; ///< UV Texture coordinates.
    float textureID;    ///< Texture slot ID (for batch rendering).
    glm::vec3 normal;   ///< Surface normal vector (for lighting and outlines).
};
