#pragma once

/// <summary>
/// Base interface for all engine services.
/// </summary>
class IService
{
public:
    virtual ~IService() = default;
    
    /// <summary>
    /// Initializes the service. Called after registration.
    /// </summary>
    virtual void Init() = 0;
    
    /// <summary>
    /// Update loop for the service. Optional.
    /// </summary>
    virtual void Update() {}
    
    /// <summary>
    /// Cleans up service resources. Called on shutdown.
    /// </summary>
    virtual void Clean() = 0;
};