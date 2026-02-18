# Engine Architecture

## Overview
The HD2D-Engine follows a **Service-Oriented Architecture** using a central `ServiceLocator`. This ensures that core systems (Rendering, Input, World) are decoupled and easily accessible without global singletons or deep dependency chains.

This allows for a "Two-Brain" architecture:
- **Macro Brain**: High-level simulation (World Generation, History, Economy).
- **Micro Brain**: Real-time gameplay (Physics, Rendering, Input).

## Core Systems

### 1. Service Locator
*   **File**: `include/Engine/Core/ServiceLocator.h`
*   **Role**: Global registry for accessing `IService` implementations.
*   **Usage**: `ServiceLocator::Get().GetService<IRenderService>()`

### 2. The UI Service
The `UIService` handles all 2D user interface rendering and interaction. It allows for data-driven UI layouts defined in JSON.

#### Layout Loading
*   **File**: `assets/ui/ui_layouts.json`
*   **Mechanism**: The `UIService` loads this JSON file on startup. It contains definitions for Screens, Widgets, and Prefabs.
*   **Hot-Reloading**: Changes to the JSON are reflected when the engine restarts (or if a reload command is triggered).

#### ScreenController Lifecycle
Screens are managed via the `BaseScreen` class (acting as the controller).
*   **OnLoad(root)**: Called when the screen is first created and the UI layout is attached.
*   **OnShow()**: Called when the screen becomes active/visible.
*   **OnHide()**: Called when the screen is hidden or replaced.
*   **OnUnload()**: Called when the screen is destroyed or the service is cleaned up.

#### Features
*   **Priority Popup Queue**: Popups can be requested with a priority level. Higher priority popups will override or queue behind lower ones.
*   **Prefab Instantiation**: Reusable UI components (like inventory slots or list items) can be defined as "Prefabs" in JSON and instantiated dynamically at runtime using `InstantiatePrefab()`.

### 3. Rendering Pipeline
*   **OpenGLRenderService**: Handles low-level OpenGL calls, mesh creation, and batched 2D rendering.
*   **PostProcessService**: Manages Framebuffer Objects (FBOs) for off-screen rendering and screen-space effects.

#### Render Loop
1.  **Update**: Input, World Generation, Physics.
2.  **Pass 1 (World)**: Render 3D Voxel World and Sprites to FBO.
3.  **Pass 2 (Post)**: Render full-screen quad with Edge Detection/Color Correction.

## Folder Structure

### `include/Engine` (Generic Framework)
*   **Core**: Base infrastructure (`IService`, `ServiceLocator`, `Sprite`, `Vertex`, `Camera`).
*   **Services/Rendering**: Graphics pipeline (`RenderService`, `OpenGLRenderService`, `ShaderService`, `PostProcessService`, `TextureAtlasService`).
*   **Services/UI**: UI system (`UIService`, `UIElement`, `BaseScreen`, `IUIService`, `UIPathRepository`).
*   **Services/Font**: Font system (`FontService`, `IFontService`, `FontPathRepository`).
*   **Services/Input**: Input handling (`InputService`, `IInputService`).
*   **Services/World**: World generation & management (`WorldService`, `MacroService`, `ChunkManager`, `BlockRegistryService`).
*   **Services/Data**: Database & resources (`DatabaseService`, `ResourceService`, `PathRegistryService`, `KenneyPathRepository`).
*   **Services/Scene**: Scene management (`SceneService`).
*   **Services/Logging**: Logging (`LoggerService`, `ILoggerService`, `LogTypes`).
*   **Utils**: Helper math and string functions.

### `include/Game` (Project Epoch Specific)
*   **World**: Voxel data structures, Chunk generation logic.
*   **Scenes**: Specific game states (MainMenu, Gameplay).
*   **UI**: Game-specific screen controllers (MainMenuController).
*   **Data**: Game configuration and asset IDs.
