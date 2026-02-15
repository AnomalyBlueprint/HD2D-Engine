#pragma once

namespace GameConfig {
    static const int RENDER_RADIUS = 2; // 1 = 3x3, 2 = 5x5, 3 = 7x7
    // Chunk Size in Pixels (16 blocks * 32 pixels)
    static const float CHUNK_PIXEL_SIZE = 512.0f;
    
    static const float MOVE_SPEED = 500.0f;
    static const float ZOOM_MIN = 0.15f;
    static const float ZOOM_MAX = 5.0f;
    static const float ZOOM_SPEED = 2.0f;
}
