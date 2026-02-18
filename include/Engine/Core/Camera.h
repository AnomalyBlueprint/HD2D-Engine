#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Services/Input/IInputService.h"

// Forward declaration if needed, or include SDL?
// The original header included <SDL.h> for SDL_ScanCode.
// Better to forward declare or include IInputService which might handle input codes indepdently?
// But InputService uses IInputService interface.
// Let's keep includes to match original functionality but cleaned up.
#include <SDL.h> 

class Camera
{
public:
    glm::vec3 position; ///< The calculated world-space position of the camera.
    glm::vec3 target;   ///< The point the camera is looking at (usually the player).
    float distance;     ///< Distance from the target.
    float height;       ///< Height offset from the target.
    float orbitAngle;   ///< Current orbit angle in radians.

    /// <summary>
    /// Constructs a new Camera with the specified aspect ratio.
    /// </summary>
    /// <param name="aspect">The aspect ratio of the viewport (width / height).</param>
    Camera(float aspect);

    /// <summary>
    /// Updates the camera position based on player movement and input.
    /// </summary>
    /// <param name="playerPos">The current position of the player.</param>
    /// <param name="deltaTime">Time elapsed since the last frame.</param>
    /// <param name="input">Input service for zoom and orbit controls.</param>
    void Update(glm::vec3 playerPos, float deltaTime, IInputService* input);

    /// <summary>
    /// Calculates the projection matrix for the camera.
    /// </summary>
    /// <returns>A 4x4 projection matrix.</returns>
    glm::mat4 GetProjectionMatrix();

    /// <summary>
    /// Calculates the view matrix for the camera.
    /// </summary>
    /// <returns>A 4x4 view matrix.</returns>
    glm::mat4 GetViewMatrix();

    /// <summary>
    /// Gets the current position of the camera.
    /// </summary>
    glm::vec3 GetPosition() { return position; }

private:
    float aspect;
};
