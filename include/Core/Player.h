#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Services/IInputService.h"
#include "Services/RenderService.h"
#include "Services/IShaderService.h"

class Player
{
public:
    Player();
    ~Player();

    void Init();
    void Update(float deltaTime, IInputService* input);
    void Render(RenderService* renderer, IShaderService* shaderService, unsigned int shaderID);

    glm::vec3 GetPosition() const { return m_position; }
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
