# Design Roadmap

## Upcoming Features (Survival Roguelike)

### 1. Physics & Interaction
*   [ ] Implement **AABB Collision Detection** for Player vs World.
*   [ ] Add **Raycasting** for block selection/breaking.
*   [ ] Implement **Gravity** and Jump mechanics.

### 2. Gameplay Loop
*   [ ] **Inventory System**: UI for holding items.
*   [ ] **Crafting**: Combine blocks/items to create new ones.
*   [ ] **Health/Stamina**: Stat management.

### 3. Audio
*   [ ] Create `IAudioService` (OpenAL or SDL_Mixer).
*   [ ] Add footstep sounds based on material type.
*   [ ] Add ambient background music.

### 4. AI & NPCs
*   [ ] Simple pathfinding (A*) for ground enemies.
*   [ ] State Machine for Enemy AI (Idle, Chase, Attack).

### 5. Advanced Rendering
*   [ ] **Shadow Mapping**: Real-time shadows from the directional light.
*   [ ] **SSAO**: Screen Space Ambient Occlusion for better depth perception.
*   [ ] **Day/Night Cycle**: Smooth transitions of skybox and light color.

## Refactoring Tasks
*   [ ] separate `Game.cpp` content into a `Scene` system.
*   [ ] Move raw OpenGL calls from `Chunk::RebuildMesh` to `RenderService` helpers.
