#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 texCoord;
    float textureID; // For batching later
    glm::vec3 normal; // Added for lighting
};
