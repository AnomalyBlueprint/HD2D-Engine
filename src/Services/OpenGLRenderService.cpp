#include "Services/OpenGLRenderService.h"
#include "Services/ServiceLocator.h"
#include "Services/ILoggerService.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <vendor/stb_image.h>


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
    log->Log("OpenGL Renderer Initialized.");

}

void OpenGLRenderService::Clear()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderService::SwapBuffers()
{
    SDL_GL_SwapWindow(window);
}

void OpenGLRenderService::Clean()
{
    if (context != nullptr)
    {
        SDL_GL_DeleteContext(context);
        context = nullptr;
    }
}
// ... Init, Clean, Clear, SwapBuffers remain the same ...

unsigned int OpenGLRenderService::CreateMesh(const std::vector<float> &vertices, const std::vector<unsigned int> &indices)
{
    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 1. Upload Vertex Data (Positions + UVs)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 2. Upload Index Data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // --- ATTRIBUTE LAYOUT ---
    // Stride is now 5 floats (x, y, z, u, v)
    int stride = 5 * sizeof(float);

    // Location 0: Position (3 floats, Offset 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(0);

    // Location 1: UV Coords (2 floats, Offset 3)
    // "void*)(3 * sizeof(float))" means "skip the first 3 floats (x,y,z) to find u,v"
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return VAO;
}

void OpenGLRenderService::DrawMesh(unsigned int meshID, int indexCount)
{
    glBindVertexArray(meshID);
    // Draw Elements (uses the internal EBO) instead of DrawArrays
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

    stbi_image_free(data); // Free CPU memory (GPU has it now)
    return textureID;
}

void OpenGLRenderService::UseTexture(unsigned int textureID)
{
    glActiveTexture(GL_TEXTURE0); // Activate Slot 0
    glBindTexture(GL_TEXTURE_2D, textureID);
}