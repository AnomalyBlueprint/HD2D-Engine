#pragma once
#include "Engine/Core/IService.h"
#include "LogTypes.h"
#include <string>
#include <functional>

// Define a callback type: A function that takes a string and a type
using LogCallback = std::function<void(const std::string &, LogType)>;

/// <summary>
/// Interface for logging messages to console or subscribers.
/// </summary>
class ILoggerService : public IService
{
public:
    virtual ~ILoggerService() = default;

    /// <summary>
    /// Logs a message with a specific severity type.
    /// </summary>
    virtual void Log(const std::string &message, LogType type) = 0;

    /// <summary>
    /// Subscribes to log events.
    /// </summary>
    virtual void Subscribe(LogCallback callback) = 0;

    /// <summary>
    /// Logs an info message.
    /// </summary>
    void Log(const std::string &message)
    {
        Log(message, LogType::Info);
    }

    /// <summary>
    /// Logs a warning message.
    /// </summary>
    void LogWarning(const std::string &message)
    {
        Log(message, LogType::Warning);
    }

    /// <summary>
    /// Logs an error message.
    /// </summary>
    void LogError(const std::string &message)
    {
        Log(message, LogType::Error);
    }
};