#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Services/IInputService.h"

// Forward declaration if needed, or include SDL?
// The original header included <SDL.h> for SDL_ScanCode.
// Better to forward declare or include IInputService which might handle input codes indepdently?
// But InputService uses IInputService interface.
// Let's keep includes to match original functionality but cleaned up.
#include <SDL.h> 

class Camera
{
public:
    // Orbit Camera Properties
    glm::vec3 position; // Calculated
    glm::vec3 target;   // Player Position
    float distance;
    float height;
    float orbitAngle;   // Radians

    Camera(float aspect);

    void Update(glm::vec3 playerPos, float deltaTime, IInputService* input);

    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();

    glm::vec3 GetPosition() { return position; }

private:
    float aspect;
};
