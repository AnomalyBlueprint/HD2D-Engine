#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    glm::vec2 position;
    float zoom;
    float aspectRatio;

    Camera(float aspect) : position(0.0f, 0.0f), zoom(1.0f), aspectRatio(aspect) {}

    // 1. Get Projection (How much world we see)
    // Zoom: Smaller number = Zoom In. Larger = Zoom Out.
    // 1. Get Projection (How much world we see)
    // Zoom: 1.0 = Default (1280x720 units visible). 0.5 = Zoom In (640x360). 2.0 = Zoom Out.
    glm::mat4 GetProjectionMatrix()
    {
        // Base resolution: 1280x720 (or whatever window size is passed)
        // Let's assume a virtual resolution of 1280 (width)
        // Height is derived from aspect ratio.
        float width = 1280.0f * zoom; 
        float height = width / aspectRatio;

        return glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -1000.0f, 1000.0f);
    }

    // 2. Get View (Where the camera is)
    glm::mat4 GetViewMatrix()
    {
        // We move the WORLD opposite to the camera
        return glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));
    }
};