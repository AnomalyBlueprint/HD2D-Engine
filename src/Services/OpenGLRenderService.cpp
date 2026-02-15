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

    // --- BATCH RENDERER SETUP ---
    // 1. Generate Buffers
    glGenVertexArrays(1, &m_batchVAO);
    glGenBuffers(1, &m_batchVBO);
    glGenBuffers(1, &m_batchEBO);

    glBindVertexArray(m_batchVAO);

    // 2. Pre-allocate VBO (Empty for now)
    glBindBuffer(GL_ARRAY_BUFFER, m_batchVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    // 3. Pre-fill EBO with indices pattern (0,1,2, 2,3,0, 4,5,6...)
    // We can just allocate a big array on stack or heap, fill it, upload, delete.
    std::vector<unsigned int> indices(MAX_INDICES);
    unsigned int offset = 0;
    for (size_t i = 0; i < MAX_INDICES; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 3;

        indices[i + 3] = offset + 1;
        indices[i + 4] = offset + 2;
        indices[i + 5] = offset + 3;

        offset += 4;
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_batchEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 4. Attributes
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

    glBindVertexArray(0);
    // ----------------------------

    // --- WHITE TEXTURE FALLBACK ---
    unsigned int whiteTex;
    glGenTextures(1, &whiteTex);
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    unsigned int whitePixel = 0xFFFFFFFF; // RGBA White
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &whitePixel);
    // CRITICAL: Must set params for texture to be "complete"
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Store it
    m_whiteTextureID = whiteTex;
    
    // Set as default current if 0
    m_currentTextureID = m_whiteTextureID; 
    
    // Bind it effectively to slot 0 to start with
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

// ... CreateMesh/DrawMesh kept for legacy/debug if needed ...
// But for now, let's keep them or remove them? The interface still demands them.
unsigned int OpenGLRenderService::CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    // Legacy support implementation
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    // Old stride layout (5 floats) -> New stride (10 floats via Vertex struct)
    // BUT Verify: CreateMesh takes vector<float>. Does the user pass packed Vertex structs?
    // Game.cpp generates `chunkVerts` via `RebuildMesh`.
    // RebuildMesh currently pushes floats.
    // It must push 10 floats per vertex to match Vertex struct.
    // So stride is sizeof(Vertex).
    
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

    // 1. Texture Wrapping (Repeat if image is smaller than mesh)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 2. Texture Filtering (Pixel Art style! Nearest Neighbor)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 3. Load Image Data
    int width, height, nrChannels;
    // OpenGL expects Y to start at bottom, images start at top. Flip it!
    stbi_set_flip_vertically_on_load(true);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        // Check if PNG (RGBA) or JPG (RGB)
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        log->Log("Texture Loaded: " + path);
    }
    else
    {
        log->LogError("Failed to load texture: " + path);
    }

    stbi_image_free(data); // Free CPU memory
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
    // Condition A: Texture Change
    if (sprite.TextureID != m_currentTextureID)
    {
        // Use 0 as "No Texture Set Yet"
        if (m_currentTextureID != 0)
        {
            Flush();
        }
        m_currentTextureID = sprite.TextureID;
    }

    // Condition B: Batch Full
    // If adding 4 vertices would exceed max
    if (m_vertices.size() + 4 > MAX_VERTICES)
    {
        Flush();
    }

    // 2. Add Quad Vertices
    // We basically transform local quad (-0.5 .. 0.5) to World Space here on CPU
    
    // Model Matrix construction (optimized)
    // T * R * S
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(sprite.Position, 0.0f));
    
    // Move origin to center for rotation, etc.
    // Our quad default is centered at 0,0 so this works fine.
    
    model = glm::rotate(model, glm::radians(sprite.Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(sprite.Size, 1.0f));

    // Default Quad Positions (Centered)
    glm::vec4 bl = model * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f); // Bottom Left
    glm::vec4 br = model * glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f); // Bottom Right
    glm::vec4 tr = model * glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f); // Top Right
    glm::vec4 tl = model * glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f); // Top Left

    // UVs
    // Default 0..1
    // If SourceRect is set, we calculate UVs.
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    // TODO: We need Texture Width/Height to normalize source rect.
    // m_textureSizes[ID] ? 
    // For now assume full texture if SourceRect is 0,0,0,0
    // OR we would need to redesign Texture Loading to store metadata.
    // For this step, let's assume Full Texture drawing.

    // Vertices order: TR, BR, BL, TL (matching indices 0,1,3, 1,2,3)
    // WAIT! Indices (0,1,2, 2,3,0) usually expects:
    // 0: Top Right, 1: Bottom Right, 2: Bottom Left, 3: Top Left
    // Let's stick to a standard counter-clockwise:
    // 0: BL, 1: BR, 2: TR, 3: TL
    // Indices: 0, 1, 2,  2, 3, 0
    // Let's re-verify indices pattern in Init.
    // indices[0]=0 (BL), [1]=1 (BR), [2]=3 (TL)  --> Triangle 1 (BL-BR-TL) - Wait..
    // Standard Quad Indices: 0, 1, 2, 2, 3, 0
    // 0=TR, 1=BR, 2=BL, 3=TL ?
    // In Init I wrote: 0,1,3, 1,2,3.
    // 0,1,3 -> TR, BR, TL
    // 1,2,3 -> BR, BL, TL
    // This forms a quad.
    
    // To match my Loop in Init (0,1,2, 2,3,0):
    // I should generate vertices in order:
    // 0: Top Right
    // 1: Bottom Right
    // 2: Bottom Left
    // 3: Top Left

    Vertex v0_tr;
    v0_tr.position = glm::vec3(tr);
    v0_tr.color = sprite.Color;
    v0_tr.texCoord = glm::vec2(u1, v1); // TR is 1,1
    v0_tr.textureID = (float)sprite.TextureID;

    Vertex v1_br;
    v1_br.position = glm::vec3(br);
    v1_br.color = sprite.Color;
    v1_br.texCoord = glm::vec2(u1, v0); // BR is 1,0
    v1_br.textureID = (float)sprite.TextureID;

    Vertex v2_bl;
    v2_bl.position = glm::vec3(bl);
    v2_bl.color = sprite.Color;
    v2_bl.texCoord = glm::vec2(u0, v0); // BL is 0,0
    v2_bl.textureID = (float)sprite.TextureID;

    Vertex v3_tl;
    v3_tl.position = glm::vec3(tl);
    v3_tl.color = sprite.Color;
    v3_tl.texCoord = glm::vec2(u0, v1); // TL is 0,1
    v3_tl.textureID = (float)sprite.TextureID;

    m_vertices.push_back(v0_tr);
    m_vertices.push_back(v1_br);
    m_vertices.push_back(v2_bl);
    m_vertices.push_back(v3_tl);
}