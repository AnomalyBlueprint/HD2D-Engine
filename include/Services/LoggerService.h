#pragma once
#include "Services/ILoggerService.h"
#include <vector>
#include <iostream>

/// <summary>
/// Standard implementation of ILoggerService using std::cout.
/// </summary>
class LoggerService : public ILoggerService
{
public:
    using ILoggerService::Log;
    void Init() override
    {
        Log("Logger Service Initialized.");
    }

    void Clean() override
    {
        listeners.clear();
    }

    void Subscribe(LogCallback callback) override
    {
        listeners.push_back(callback);
    }

    void Log(const std::string &message, LogType type) override
    {
        switch (type)
        {
        case LogType::Info:
            std::cout << "[INFO] ";
            break;
        case LogType::Warning:
            std::cout << "\033[33m[WARN]\033[0m ";
            break; // Yellow
        case LogType::Error:
            std::cout << "\033[31m[ERR] \033[0m ";
            break; // Red
        case LogType::Gameplay:
            std::cout << "\033[32m[GAME]\033[0m ";
            break; // Green
        }
        std::cout << message << std::endl;

        for (const auto &callback : listeners)
        {
            callback(message, type);
        }
    }

private:
    std::vector<LogCallback> listeners;
};