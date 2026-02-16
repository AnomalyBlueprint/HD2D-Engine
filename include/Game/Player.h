#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Engine/Services/IInputService.h"
#include "Engine/Services/RenderService.h"
#include "Engine/Services/IShaderService.h"

/// <summary>
/// Represents the player character in the world.
/// Handles movement, rotation, and rendering of the player mesh.
/// </summary>
class Player
{
public:
    Player();
    ~Player();

    /// <summary>
    /// Initializes the player mesh resources.
    /// </summary>
    void Init();

    /// <summary>
    /// Updates the player state, handling input for movement and rotation.
    /// </summary>
    /// <param name="deltaTime">Time elapsed since the last frame.</param>
    /// <param name="input">Input service for reading key states.</param>
    void Update(float deltaTime, IInputService* input);

    /// <summary>
    /// Renders the player mesh.
    /// </summary>
    /// <param name="renderer">Render service used to draw the mesh.</param>
    /// <param name="shaderService">Shader service used to set uniforms.</param>
    /// <param name="shaderID">ID of the shader program to use.</param>
    void Render(RenderService* renderer, IShaderService* shaderService, unsigned int shaderID);

    /// <summary>
    /// Gets the current world position of the player.
    /// </summary>
    glm::vec3 GetPosition() const { return m_position; }

    /// <summary>
    /// Sets the world position of the player.
    /// </summary>
    void SetPosition(glm::vec3 pos) { m_position = pos; }

private:
    glm::vec3 m_position;
    float m_yaw; // Rotation in degrees around Y axis
    float m_moveSpeed;
    float m_turnSpeed;

    // Mesh IDs
    unsigned int m_bodyMeshID;
    unsigned int m_faceMeshID;
    unsigned int m_bodyIndexCount;
    unsigned int m_faceIndexCount;

    // Helper to create a cube mesh
    void CreateCubeMesh(float width, float height, float depth, glm::vec4 color, unsigned int& meshID, unsigned int& indexCount);
};
