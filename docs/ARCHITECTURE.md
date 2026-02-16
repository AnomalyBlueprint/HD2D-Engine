# Engine Architecture

## Overview
The HD2D-Engine follows a **Service-Oriented Architecture** using a central `ServiceLocator`. This ensures that core systems (Rendering, Input, World) are decoupled and easily accessible without global singletons or deep dependency chains.

## Core Systems

### 1. Service Locator
*   **File**: `Services/ServiceLocator.h`
*   **Role**: Global registry for accessing `IService` implementations.
*   **Usage**: `ServiceLocator::Get().GetService<IRenderService>()`

### 2. Rendering Pipeline
*   **OpenGLRenderService**: Handles low-level OpenGL calls, mesh creation, and batched 2D rendering.
*   **OpenGLShaderService**: Manages shader compilation, linking, and uniform updates.
*   **PostProcessService**: Manages Framebuffer Objects (FBOs) for off-screen rendering and screen-space effects (Edge Detection).

#### Render Loop
1.  **Update**: Input, World Generation (Chunks), Player Physics.
2.  **Pass 1 (World)**: Render the 3D Voxel World and Sprites to an FBO (Color + Normal Buffers).
3.  **Pass 2 (Post)**: Render a full-screen quad using the data from Pass 1 to apply Edge Detection and Color Correction.

### 3. World Generation
*   **Chunk**: Represents a 16x16x16 block volume. Handles mesh generation with face culling (only visible faces are generated).
*   **WorldService**: Uses `FastNoiseLite` to generate terrain heightmaps. Implements "Terraced" generation for a defined voxel look.
*   **BlockRegistryService**: Decouples Block IDs (byte) from Texture Assets (KenneyIDs).

### 4. Coordinate System
*   **Y-Up**: The world uses Y as the vertical axis.
*   **Units**: 1 Unit = 1 Voxel.
*   **Top-Left Origin (UVs)**: Texture coordinates follow standard OpenGL (0,0 bottom-left) but are flipped during loading/packing to match logical top-left expectations where needed.

## Directory Structure
*   `src/Core`: Basic entities (Player, Camera, Vertex).
*   `src/Services`: System implementations.
*   `src/World`: Voxel data structures.
*   `include/Data`: Asset IDs and Configs.
