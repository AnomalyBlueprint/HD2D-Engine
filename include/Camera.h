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
    glm::mat4 GetProjectionMatrix()
    {
        float height = 2.0f * zoom; // Base height is 2.0 (-1 to 1)
        float width = height * aspectRatio;

        return glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, -1.0f, 1.0f);
    }

    // 2. Get View (Where the camera is)
    glm::mat4 GetViewMatrix()
    {
        // We move the WORLD opposite to the camera
        return glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));
    }
};