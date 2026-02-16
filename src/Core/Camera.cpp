#include "Core/Camera.h"
#include <cmath>

Camera::Camera(float aspect) 
    : distance(20.0f), height(15.0f), orbitAngle(0.0f), aspect(aspect) 
{
}

void Camera::Update(glm::vec3 playerPos, float deltaTime, IInputService* input)
{
    target = playerPos;

    // Orbit Rotation (Q / E)
    if (input->IsKeyDown(SDL_SCANCODE_Q)) orbitAngle -= 2.0f * deltaTime;
    if (input->IsKeyDown(SDL_SCANCODE_E)) orbitAngle += 2.0f * deltaTime;

    // Zoom (Scroll)
    int scroll = input->GetMouseScroll();
    if (scroll != 0)
    {
        distance -= scroll * 100.0f; // Faster zoom
        if (distance < 50.0f) distance = 50.0f;
        if (distance > 3000.0f) distance = 3000.0f;
    }

    // Calculate Position
    // Look down at 45 degrees: Height should match Distance roughly
    height = distance;

    float camX = target.x - (sin(orbitAngle) * distance);
    float camZ = target.z - (cos(orbitAngle) * distance);
    position = glm::vec3(camX, target.y + height, camZ);
}

glm::mat4 Camera::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10000.0f);
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}
