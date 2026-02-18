# Game Design Specifications

This document serves as the consolidated source of truth for Project Epoch's game design, merging the original Master Design Document and Pixel Forge Specifications.

## 1. World Topology

### Macro Scale
The world is generated as a massive **256x256 Macro Chunk** grid.
Each "Macro Chunk" represents a distinct region of the world with its own biome, political affiliation, and resource data.

*   **Total Size**: 65,536 Macro Chunks.
*   **Micro Detail**: Each Macro Chunk contains thousands of individual tiles (voxels).
*   **Total Area**: Approximately **17.1 Billion** addressable tiles.

### Two-Brain Simulation
The engine uses a "Two-Brain" architecture to manage this scale:
*   **Macro Brain**: Simulates high-level data (Trade routes, Wars, weather patterns) across the entire 256x256 grid abstractly.
*   **Micro Brain**: Generates the actual 3D voxel geometry and physics only for the chunks immediately surrounding the player.

## 2. The Pixel Forge (Weapon Generation)

The **Pixel Forge** is the system responsible for procedurally generating weapons. It uses a **4-Quadrant Data Map** to define every item.

### The 4 Quadrants
A weapon is not just a sprite; it is a composite of 4 data layers:

1.  **Visuals (Top-Left)**: The sprite data, particle effects, and animation frames.
2.  **Physics (Top-Right)**: Hitboxes, weight, swing speed, and recoil data.
3.  **Logic (Bottom-Left)**: Scripted behaviors (e.g., "OnHit: Apply Burn", "OnEquip: +5 STR").
4.  **Metadata (Bottom-Right)**: Lore text, crafting material requirements, and rarity flags.

### Crafting Process
When a player crafts a weapon, the Forge:
1.  Selects a base template (e.g., "Sword").
2.  overlays material properties (e.g., "Iron" vs "Void Metal") affecting the Physics quadrant.
3.  Injects a "Soul" or "Rune" which populates the Logic quadrant.
4.  Generates the Visuals based on the combination of parts.

## 3. Bestiary & Inference Engine

Creatures in Project Epoch are not hard-coded classes. They are generated via an **Inference Engine**.

### Logic
Instead of manually setting "HP = 50" for a Goblin, we define:
*   **Base Species**: Goblin (Small, Fast, Low Const).
*   **Role**: Scout (Light Armor, Dagger).
*   **Level**: 5.

The Engine *interferes* the final stats:
`HP = (Base.Con * Level) * Role.Modifier`

### Data flow
1.  **Designers** use the `BestiaryArchitect.html` tool to tweak Base stats and Role modifiers.
2.  **Export**: The tool calculates the spread and exports a balanced SQL database (`assets/database/epoch.db`).
3.  **Runtime**: The game queries this DB to spawn enemies.
