#include "Services/OpenGLRenderService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <vendor/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

OpenGLRenderService::OpenGLRenderService(SDL_Window *win) : window(win) {}

OpenGLRenderService::~OpenGLRenderService()
{
    Clean();
}

void OpenGLRenderService::Init()
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();
    
    // 1. Create Context
    context = SDL_GL_CreateContext(window);
    if (!context)
    {
        log->LogError("Failed to create OpenGL context: " + std::string(SDL_GetError()));
        return;
    }

    // 2. Init GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        log->LogError("Failed to initialize GLEW: " + std::string((const char *)glewGetErrorString(err)));
        return;
    }

    // 3. Setup
    glViewport(0, 0, 1280, 720);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    // --- BATCH RENDERER SETUP ---
    glGenVertexArrays(1, &m_batchVAO);
    glGenBuffers(1, &m_batchVBO);
    glGenBuffers(1, &m_batchEBO);

    glBindVertexArray(m_batchVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_batchVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // Pre-fill EBO with indices pattern (0,1,2, 2,3,0, 4,5,6...)
    std::vector<unsigned int> indices(MAX_INDICES);
    unsigned int offset = 0;
    for (size_t i = 0; i < MAX_INDICES; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2; 

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;

        offset += 4;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_batchEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Attributes
    // Position (Vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // Color (Vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    // TexCoord (Vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // TextureID (Float)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureID));

    // Normal
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);

    // --- WHITE TEXTURE FALLBACK ---
    unsigned int whiteTex;
    glGenTextures(1, &whiteTex);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    unsigned int whitePixel = 0xFFFFFFFF; // RGBA White
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_whiteTextureID = whiteTex;
    m_currentTextureID = m_whiteTextureID; 
    glBindTexture(GL_TEXTURE_2D, m_whiteTextureID); 

    log->Log("OpenGL Batched Renderer Initialized.");
}

void OpenGLRenderService::Clear()
{
    glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderService::SwapBuffers()
{
    SDL_GL_SwapWindow(window);
}

void OpenGLRenderService::Clean()
{
    if (m_batchVAO != 0) glDeleteVertexArrays(1, &m_batchVAO);
    if (m_batchVBO != 0) glDeleteBuffers(1, &m_batchVBO);
    if (m_batchEBO != 0) glDeleteBuffers(1, &m_batchEBO);

    if (context != nullptr)
    {
        SDL_GL_DeleteContext(context);
        context = nullptr;
    }
}

// Helper methods for Mesh Creation and Drawing
// Required by RenderService interface
unsigned int OpenGLRenderService::CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    int stride = sizeof(Vertex); 
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, position));
    
    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, color));
    
    // TexCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, texCoord));

    // TexID
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, textureID));

    // Normal
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void *)offsetof(Vertex, normal));

    glBindVertexArray(0);
    return VAO;
}

void OpenGLRenderService::DrawMesh(unsigned int meshID, int indexCount)
{
    glBindVertexArray(meshID);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

unsigned int OpenGLRenderService::LoadTexture(const std::string &path)
{
    auto log = ServiceLocator::Get().GetService<ILoggerService>();

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        log->Log("Texture Loaded: " + path);
    }
    else
    {
        log->LogError("Failed to load texture: " + path);
    }

    stbi_image_free(data);
    return textureID;
}

void OpenGLRenderService::UseTexture(unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0); // Activate Slot 0
    
    // Fallback to White Texture if 0 is passed
    if (textureID == 0)
    {
        textureID = m_whiteTextureID;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureID);
}

// --- BATCH RENDERER IMPLEMENTATION ---

void OpenGLRenderService::Begin()
{
    m_vertices.clear();
    m_currentTextureID = 0;
}

void OpenGLRenderService::End()
{
    // Draw whatever is left
    Flush();
}

void OpenGLRenderService::Flush()
{
    if (m_vertices.empty()) return;

    if (m_currentTextureID != 0)
    {
        UseTexture(m_currentTextureID);
    }

    glBindVertexArray(m_batchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_batchVBO);
    
    // SubData -> Update only the part we used
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertices.size() * sizeof(Vertex), m_vertices.data());

    // Calculate how many indices to draw
    // 4 vertices = 6 indices. (vertices / 4) * 6
    size_t indexCount = (m_vertices.size() / 4) * 6;

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    m_vertices.clear();
}

void OpenGLRenderService::DrawSprite(const Sprite& sprite)
{
    // 1. Check Flush Conditions
    if (sprite.TextureID != m_currentTextureID)
    {
        if (m_currentTextureID != 0)
        {
            Flush();
        }
        m_currentTextureID = sprite.TextureID;
    }

    if (m_vertices.size() + 4 > MAX_VERTICES)
    {
        Flush();
    }

    // 2. Add Quad Vertices
    
    // Model Matrix construction
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(sprite.Position, 0.0f));
    model = glm::rotate(model, glm::radians(sprite.Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(sprite.Size, 1.0f));

    // Default Quad Positions (Centered 1x1)
    glm::vec4 bl = model * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f); // Bottom Left
    glm::vec4 br = model * glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f); // Bottom Right
    glm::vec4 tr = model * glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f); // Top Right
    glm::vec4 tl = model * glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f); // Top Left

    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    // Ordered to match indices: 0,1,2, 2,3,0 (Counter-Clockwise)
    // 0: Bottom Left
    // 1: Bottom Right
    // 2: Top Right
    // 3: Top Left

    Vertex v0_bl;
    v0_bl.position = glm::vec3(bl);
    v0_bl.color = sprite.Color;
    v0_bl.texCoord = glm::vec2(u0, v0);
    v0_bl.textureID = (float)sprite.TextureID;
    v0_bl.normal = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v1_br;
    v1_br.position = glm::vec3(br);
    v1_br.color = sprite.Color;
    v1_br.texCoord = glm::vec2(u1, v0);
    v1_br.textureID = (float)sprite.TextureID;
    v1_br.normal = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v2_tr;
    v2_tr.position = glm::vec3(tr);
    v2_tr.color = sprite.Color;
    v2_tr.texCoord = glm::vec2(u1, v1);
    v2_tr.textureID = (float)sprite.TextureID;
    v2_tr.normal = glm::vec3(0.0f, 0.0f, 1.0f);

    Vertex v3_tl;
    v3_tl.position = glm::vec3(tl);
    v3_tl.color = sprite.Color;
    v3_tl.texCoord = glm::vec2(u0, v1);
    v3_tl.textureID = (float)sprite.TextureID;
    v3_tl.normal = glm::vec3(0.0f, 0.0f, 1.0f);

    m_vertices.push_back(v0_bl);
    m_vertices.push_back(v1_br);
    m_vertices.push_back(v2_tr);
    m_vertices.push_back(v3_tl);
}