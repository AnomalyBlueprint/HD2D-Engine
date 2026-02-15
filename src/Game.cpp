#include "Game.h"
#include "Services/ServiceLocator.h"
#include "Services/RenderService.h"
#include "Services/OpenGLRenderService.h"
#include "Services/ILoggerService.h"
#include "Services/LogTypes.h"
#include "Services/LoggerService.h"
#include "Services/IShaderService.h"
#include "Services/OpenGLShaderService.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Game::Init()
{
    // 1. Logger (First!)
    auto loggerServ = std::make_shared<LoggerService>();
    ServiceLocator::Get().Register<ILoggerService>(loggerServ);

    // Helper lambda
    auto logger = []()
    { return ServiceLocator::Get().GetService<ILoggerService>(); };

    logger()->Log("Engine Starting...");

    // 2. SDL Init
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        logger()->LogError("SDL Init Failed!");
        return;
    }

    // 3. OpenGL Settings & Window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("HD-2D Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    // --- CAMERA SETUP ---
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    // Safety check
    if (h == 0)
        h = 1;
    float aspect = (float)w / (float)h;

    camera = std::make_shared<Camera>(aspect);
    camera->zoom = 2.0f; // Zoom out a bit to see more
    // --------------------

    // 4. Render Service (CRITICAL: This initializes GLEW!)
    auto renderer = std::make_shared<OpenGLRenderService>(window);
    ServiceLocator::Get().Register<RenderService>(renderer);

    // 5. NOW you can use Shaders (Context is ready)
    auto shaderSystem = std::make_shared<OpenGLShaderService>();
    ServiceLocator::Get().Register<IShaderService>(shaderSystem);

    // Test It
    auto shaders = ServiceLocator::Get().GetService<IShaderService>();
    basicShaderID = shaders->LoadShader("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    // Note: Make sure this path exists on your machine!
    textureID = renderer->LoadTexture("assets/textures/kenney_retro-textures-1/PNG/floor_ground_dirt.png");

    // 1. Define 4 Vertices (A Quad)
    std::vector<float> vertices = {
        // Positions          // Texture Coords
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // Top Right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom Left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // Top Left
    };

    // 2. Define Indices (Order of drawing)
    std::vector<unsigned int> indices = {
        0, 1, 3,
        1, 2, 3};

    // 3. Create Mesh
    triangleMeshID = renderer->CreateMesh(vertices, indices);

    logger()->Log("Quad Mesh Created! ID: " + std::to_string(triangleMeshID));

    isRunning = true;
    logger()->Log("Game Initialized.");
}

void Game::Run()
{
    while (isRunning)
    {
        // 1. Input
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                isRunning = false;
        }

        // 2. Render Services
        auto renderer = ServiceLocator::Get().GetService<RenderService>();
        auto shaders = ServiceLocator::Get().GetService<IShaderService>();

        // 3. Simple Camera Movement (Arrow Keys / WASD)
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W])
            camera->position.y += 0.01f;
        if (state[SDL_SCANCODE_S])
            camera->position.y -= 0.01f;
        if (state[SDL_SCANCODE_A])
            camera->position.x -= 0.01f;
        if (state[SDL_SCANCODE_D])
            camera->position.x += 0.01f;

        // 4. Update Shader Matrices & Render
        if (renderer)
        {
            renderer->Clear();

            if (triangleMeshID > 0 && basicShaderID > 0)
            {
                shaders->UseShader(basicShaderID);

                // Update Camera Matrices
                shaders->SetMat4(basicShaderID, "projection", camera->GetProjectionMatrix());
                shaders->SetMat4(basicShaderID, "view", camera->GetViewMatrix());

                renderer->UseTexture(textureID);
                renderer->DrawMesh(triangleMeshID, 6);
            }
            renderer->SwapBuffers();
        }
    }
}

void Game::Clean()
{
    ServiceLocator::Get().Clean();
    SDL_DestroyWindow(window);
    SDL_Quit();
}