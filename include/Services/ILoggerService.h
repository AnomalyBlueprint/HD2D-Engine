#pragma once
#include "IService.h"
#include "LogTypes.h"
#include <string>
#include <functional>

// Define a callback type: A function that takes a string and a type
using LogCallback = std::function<void(const std::string &, LogType)>;

class ILoggerService : public IService
{
public:
    virtual ~ILoggerService() = default;

    // Send a message
    virtual void Log(const std::string &message, LogType type) = 0;

    // Listen for messages (For UI/Console)
    virtual void Subscribe(LogCallback callback) = 0;

    // --- UNITY STYLE HELPERS (Implemented Here) ---

    // 1. Log (Info)
    void Log(const std::string &message)
    {
        Log(message, LogType::Info);
    }

    // 2. LogWarning
    void LogWarning(const std::string &message)
    {
        Log(message, LogType::Warning);
    }

    // 3. LogError
    void LogError(const std::string &message)
    {
        Log(message, LogType::Error);
    }
};