#pragma once

/// <summary>
/// Severity levels for logging.
/// </summary>
enum class LogType
{
    Info,    ///< System messages (e.g., "Engine Started")
    Warning, ///< Non-fatal issues (e.g., "Texture not found")
    Error,   ///< Crashes/Critical (e.g., "OpenGL failed")
    Gameplay ///< In-Game Events (e.g., "Player hit Enemy")
};