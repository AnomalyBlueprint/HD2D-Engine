#pragma once

/// <summary>
/// Base interface for all engine services.
/// </summary>
class IService
{
private:
    bool m_isInitialized = false;

protected:
    /// <summary>
    /// Actual initialization logic to be overridden by derived services.
    /// </summary>
    virtual void OnInitialize() = 0;

public:
    virtual ~IService() = default;

    /// <summary>
    /// Public entry point. Ensures initialization only happens once.
    /// </summary>
    void Init()
    {
        if (m_isInitialized)
            return;

        OnInitialize();
        m_isInitialized = true;
    }

    /// <summary>
    /// Update loop for the service. Optional.
    /// </summary>
    virtual void Update() {}

    /// <summary>
    /// Cleans up service resources. Called on shutdown.
    /// </summary>
    virtual void Clean() = 0;
};