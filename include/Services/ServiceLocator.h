#pragma once
#include "Services/IService.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>

/// <summary>
/// Global registry for accessing engine services.
/// Implements the Service Locator pattern.
/// </summary>
class ServiceLocator
{
public:
    /// <summary>
    /// Gets the singleton instance of the ServiceLocator.
    /// </summary>
    static ServiceLocator &Get()
    {
        static ServiceLocator instance;
        return instance;
    }

    /// <summary>
    /// Registers a service implementation.
    /// </summary>
    /// <typeparam name="T">The service type (interface).</typeparam>
    /// <param name="service">Shared pointer to the service instance.</param>
    template <typename T>
    void Register(std::shared_ptr<T> service)
    {
        services[std::type_index(typeid(T))] = service;
        service->Init();
    }

    /// <summary>
    /// Retrieves a registered service.
    /// </summary>
    /// <typeparam name="T">The service type (interface) to retrieve.</typeparam>
    /// <returns>Shared pointer to the service, or nullptr if not found.</returns>
    template <typename T>
    std::shared_ptr<T> GetService()
    {
        auto it = services.find(std::type_index(typeid(T)));
        if (it == services.end())
        {
            std::cerr << "[ServiceLocator] Critical: Service not found!" << std::endl;
            return nullptr;
        }
        return std::static_pointer_cast<T>(it->second);
    }

    /// <summary>
    /// Cleans up all registered services.
    /// </summary>
    void Clean()
    {
        for (auto &pair : services)
        {
            pair.second->Clean();
        }
        services.clear();
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<IService>> services;
    ServiceLocator() {}
};