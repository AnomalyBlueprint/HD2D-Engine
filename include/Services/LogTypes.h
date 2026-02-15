#pragma once

enum class LogType
{
    Info,    // System messages (e.g., "Engine Started")
    Warning, // Non-fatal issues (e.g., "Texture not found, using default")
    Error,   // Crashes/Critical (e.g., "OpenGL Context failed")
    Gameplay // In-Game Events (e.g., "Player hit Enemy")
};