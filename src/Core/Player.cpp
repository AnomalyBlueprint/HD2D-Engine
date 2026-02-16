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

    // Debug Movement (Up/Down)
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

    // 24 Vertices (4 per face * 6 faces) for Flat Shading
    std::vector<float> vertices;
    
    // Helper to push a vertex
    auto pushVert = [&](glm::vec3 p, glm::vec3 n, float u, float v) {
        vertices.insert(vertices.end(), { p.x, p.y, p.z, color.r, color.g, color.b, color.a, u, v, 0.0f, n.x, n.y, n.z });
    };

    // Front (+Z)
    glm::vec3 nFront(0, 0, 1);
    pushVert({-w, -h, d}, nFront, 0, 0); // BL
    pushVert({ w, -h, d}, nFront, 1, 0); // BR
    pushVert({ w,  h, d}, nFront, 1, 1); // TR
    pushVert({-w,  h, d}, nFront, 0, 1); // TL

    // Back (-Z)
    glm::vec3 nBack(0, 0, -1);
    pushVert({ w, -h, -d}, nBack, 0, 0); // BR
    pushVert({-w, -h, -d}, nBack, 1, 0); // BL
    pushVert({-w,  h, -d}, nBack, 1, 1); // TL
    pushVert({ w,  h, -d}, nBack, 0, 1); // TR

    // Left (-X)
    glm::vec3 nLeft(-1, 0, 0);
    pushVert({-w, -h, -d}, nLeft, 0, 0); // BL Back
    pushVert({-w, -h,  d}, nLeft, 1, 0); // BL Front
    pushVert({-w,  h,  d}, nLeft, 1, 1); // TL Front
    pushVert({-w,  h, -d}, nLeft, 0, 1); // TL Back

    // Right (+X)
    glm::vec3 nRight(1, 0, 0);
    pushVert({ w, -h,  d}, nRight, 0, 0); // BR Front
    pushVert({ w, -h, -d}, nRight, 1, 0); // BR Back
    pushVert({ w,  h, -d}, nRight, 1, 1); // TR Back
    pushVert({ w,  h,  d}, nRight, 0, 1); // TR Front

    // Top (+Y)
    glm::vec3 nTop(0, 1, 0);
    pushVert({-w, h,  d}, nTop, 0, 0); // TL Front
    pushVert({ w, h,  d}, nTop, 1, 0); // TR Front
    pushVert({ w, h, -d}, nTop, 1, 1); // TR Back
    pushVert({-w, h, -d}, nTop, 0, 1); // TL Back

    // Bottom (-Y)
    glm::vec3 nBottom(0, -1, 0);
    pushVert({-w, -h, -d}, nBottom, 0, 0); // BL Back
    pushVert({ w, -h, -d}, nBottom, 1, 0); // BR Back
    pushVert({ w, -h,  d}, nBottom, 1, 1); // BR Front
    pushVert({-w, -h,  d}, nBottom, 0, 1); // BL Front

    // Indices (0,1,2, 2,3,0 for each face)
    std::vector<unsigned int> indices;
    for (int i = 0; i < 6; i++) {
        int start = i * 4;
        indices.push_back(start + 0);
        indices.push_back(start + 1);
        indices.push_back(start + 2);
        indices.push_back(start + 2);
        indices.push_back(start + 3);
        indices.push_back(start + 0);
    }

    indexCount = indices.size();
    auto renderer = ServiceLocator::Get().GetService<RenderService>();
    if (renderer)
    {
        meshID = renderer->CreateMesh(vertices, indices);
    }
}