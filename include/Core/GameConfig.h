#pragma once

namespace GameConfig {
    static const int RENDER_RADIUS = 2; // 1 = 3x3, 2 = 5x5, 3 = 7x7
    // Chunk Size in Pixels (32 blocks * 32 pixels)
    static const float CHUNK_PIXEL_SIZE = 1024.0f;
    static constexpr float DAY_CYCLE_DURATION = 20.0f;
    
    static const float MOVE_SPEED = 500.0f;
    static const float ZOOM_MIN = 0.15f;
    static const float ZOOM_MAX = 5.0f;
    static const float ZOOM_SPEED = 2.0f;

    // Alias for user preference
    static constexpr float DAY_CYCLE_SPEED = 20.0f; 

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
