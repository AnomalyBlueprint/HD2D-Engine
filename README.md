# Project Epoch (HD2D Engine)

**Project Epoch** is a history-first roguelike engine written in C++17, designed to explore HD-2D aesthetics using OpenGL and SDL2. It features a hybrid 2D/3D rendering pipeline, shader-based lighting, and voxel world generation.

## Quick Start

### Build
```bash
make
```

### Run
```bash
./bin/engine_3d
```

## Directory Map

- **`src/Engine`**: The generic game engine framework (Rendering, Input, UI, Services).
- **`src/Game`**: The specific gameplay logic for Project Epoch (Player, World, Game Config).
- **`include/`**: Header files mirroring the src structure.
- **`tools/`**: Custom HTML/JS design tools for UI and Data.
- **`assets/`**: Game assets (textures, shaders, database, layouts).
- **`docs/`**: Project documentation.

## Tools

Custom design tools are provided in the `tools/` directory to facilitate content creation without hard-coding:
- **UI Architect**: Visual editor for UI layouts.
- **Bestiary Architect**: Tool for balancing and generating creature data.

See [docs/TOOLS_WORKFLOW.md](docs/TOOLS_WORKFLOW.md) for usage instructions.
