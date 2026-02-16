# HD2D Engine

A custom C++ Game Engine built from scratch using SDL2 and OpenGL, featuring a high-definition 2D aesthetic with 3D environments, inspired by Octopath Traveler and Minecraft.

## Key Features
*   **Voxel World Generation**: Procedural terrain using Simplex Noise (FastNoiseLite).
*   **Hybrid Rendering**: 2D Sprites in a 3D World (Billboarding).
*   **Cel Shading**: Custom shader with configurable bands, rim lighting, and outlines.
*   **Post-Processing**: Screen-space edge detection (Sobel filter) for stylized outlines.
*   **Service Locator Architecture**: Decoupled systems for Input, Rendering, Resources, and more.
*   **Dynamic Visual Styles**: Switch between "Minecraft", "Borderlands", and "Mo.co" logic at runtime.

## Controls
*   **WASD**: Move Player
*   **Space**: Jump (Physics pending)
*   **Esc**: Exit Game
*   **1**: Switch to "Minecraft" Style (No outlines, flat shading)
*   **2**: Switch to "Borderlands" Style (Heavy outlines, 2-band cel shading)
*   **3**: Switch to "Mo.co" Style (Softer outlines, vibrant colors)

## Asset Credits
*   **Kenney Assets**: Rogue-like Indoor/Outdoor packs (CC0).
*   **FastNoiseLite**: MIT License.

## Build Instructions
1.  Ensure you have `SDL2`, `GLEW`, and `GLM` installed (mapped via `xmake` or system paths).
2.  Run `make` to build.
3.  Run `./engine_3d` to start the engine.
