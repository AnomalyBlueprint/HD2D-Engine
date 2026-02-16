#pragma once

/// <summary>
/// Global configuration constants for the game engine.
/// </summary>
namespace GameConfig {
    static const int RENDER_RADIUS = 2; ///< Chunk render radius (e.g., 2 means 5x5 grid).
    
    /// <summary>
    /// Size of a chunk in world pixels (32 blocks * 32 pixels/block).
    /// </summary>
    static const float CHUNK_PIXEL_SIZE = 1024.0f;
    static constexpr float DAY_CYCLE_DURATION = 20.0f;
    
    static const float MOVE_SPEED = 500.0f;
    static const float ZOOM_MIN = 0.15f;
    static const float ZOOM_MAX = 5.0f;
    static const float ZOOM_SPEED = 2.0f;

    static constexpr float DAY_CYCLE_SPEED = 20.0f; 

    /// <summary>
    /// Configuration for Cel-Shading and Post-Processing effects.
    /// </summary>
    struct CelShading {
        static constexpr bool ENABLED = true;
        
        // Outlines
        static constexpr bool OUTLINES_ENABLED = true;
        static constexpr float OUTLINE_WIDTH = 0.001f;   // UV Space width
        
        // Lighting
        static constexpr float AMBIENT_STRENGTH = 0.4f;
        static constexpr int CEL_BANDS = 3;             // 3 Levels of light
    };
}
