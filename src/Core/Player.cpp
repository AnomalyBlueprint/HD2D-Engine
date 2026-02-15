#include "Core/Player.h"
#include "Services/ServiceLocator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

Player::Player() 
    : m_position(0.0f, 200.0f, 0.0f), m_yaw(0.0f), m_moveSpeed(200.0f), m_turnSpeed(120.0f),
      m_bodyMeshID(0), m_faceMeshID(0), m_bodyIndexCount(0), m_faceIndexCount(0)
{
}

Player::~Player()
{
}

void Player::Init()
{
    // Body: Red Cube (Size 24x32x24)
    CreateCubeMesh(24.0f, 32.0f, 24.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), m_bodyMeshID, m_bodyIndexCount);

    // Face: Yellow Cube (offset forward)
    CreateCubeMesh(16.0f, 10.0f, 10.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), m_faceMeshID, m_faceIndexCount);
}

void Player::Update(float deltaTime, IInputService* input)
{
    if (!input) return;

    // Rotation
    if (input->IsKeyDown(SDL_SCANCODE_A)) m_yaw += m_turnSpeed * deltaTime;
    if (input->IsKeyDown(SDL_SCANCODE_D)) m_yaw -= m_turnSpeed * deltaTime;

    // Movement
    float yawRad = glm::radians(m_yaw);
    glm::vec3 forward;
    forward.x = sin(yawRad); 
    forward.y = 0.0f;
    forward.z = cos(yawRad);
    forward = glm::normalize(forward); 

    if (input->IsKeyDown(SDL_SCANCODE_W)) m_position += forward * m_moveSpeed * deltaTime;
    if (input->IsKeyDown(SDL_SCANCODE_S)) m_position -= forward * m_moveSpeed * deltaTime; 

    // Lock Height (Terrain max is ~640. Fly at 800)
    // m_position.y = 800.0f; 

    // lets add controls for z and c to go up and down in y axis too.
    if (input->IsKeyDown(SDL_SCANCODE_Z)) m_position.y += m_moveSpeed * deltaTime;
    if (input->IsKeyDown(SDL_SCANCODE_C)) m_position.y -= m_moveSpeed * deltaTime;
}

void Player::Render(RenderService* renderer, IShaderService* shaderService, unsigned int shaderID)
{
    if (!renderer || !shaderService) return;

    // Draw Body
    glm::mat4 modelBody = glm::mat4(1.0f);
    modelBody = glm::translate(modelBody, m_position);
    modelBody = glm::rotate(modelBody, glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    
    shaderService->SetMat4(shaderID, "model", modelBody);
    renderer->DrawMesh(m_bodyMeshID, m_bodyIndexCount);

    // Draw Face
    glm::mat4 modelFace = modelBody; 
    modelFace = glm::translate(modelFace, glm::vec3(0.0f, 5.0f, 12.0f)); 
    
    shaderService->SetMat4(shaderID, "model", modelFace);
    renderer->DrawMesh(m_faceMeshID, m_faceIndexCount);
}

void Player::CreateCubeMesh(float width, float height, float depth, glm::vec4 color, unsigned int& meshID, unsigned int& indexCount)
{
    float w = width * 0.5f;
    float h = height * 0.5f;
    float d = depth * 0.5f;

    // 8 Vertices
    std::vector<float> vertices = {
        // Positions(3) + Color(4) + TexCoord(2) + TexID(1)
        -w, -h,  d,  color.r, color.g, color.b, color.a, 0.0f, 0.0f, 0.0f, // 0: BL Front
         w, -h,  d,  color.r, color.g, color.b, color.a, 1.0f, 0.0f, 0.0f, // 1: BR Front
         w,  h,  d,  color.r, color.g, color.b, color.a, 1.0f, 1.0f, 0.0f, // 2: TR Front
        -w,  h,  d,  color.r, color.g, color.b, color.a, 0.0f, 1.0f, 0.0f, // 3: TL Front
        -w, -h, -d,  color.r, color.g, color.b, color.a, 1.0f, 0.0f, 0.0f, // 4: BL Back
         w, -h, -d,  color.r, color.g, color.b, color.a, 0.0f, 0.0f, 0.0f, // 5: BR Back
         w,  h, -d,  color.r, color.g, color.b, color.a, 0.0f, 1.0f, 0.0f, // 6: TR Back
        -w,  h, -d,  color.r, color.g, color.b, color.a, 1.0f, 1.0f, 0.0f  // 7: TL Back
    };

    // 36 Indices (CCW Winding Order)
    std::vector<unsigned int> indices = {
        // Front (Normal +Z)
        0, 1, 2, 2, 3, 0,
        // Back (Normal -Z)
        5, 4, 7, 7, 6, 5,
        // Left (Normal -X)
        4, 0, 3, 3, 7, 4,
        // Right (Normal +X)
        1, 5, 6, 6, 2, 1,
        // Top (Normal +Y) - FIXED WINDING (Was CW, now CCW)
        3, 2, 6, 6, 7, 3, 
        // Bottom (Normal -Y) - FIXED WINDING
        4, 5, 1, 1, 0, 4
    };

    indexCount = indices.size();
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    if (renderer)
    {
        meshID = renderer->CreateMesh(vertices, indices);
    }
}