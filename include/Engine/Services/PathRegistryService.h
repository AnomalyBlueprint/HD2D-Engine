#pragma once
#include "Engine/Services/IService.h"
#include "Engine/Services/IPathRepository.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>

/// <summary>
/// Service for managing multiple path repositories.
/// Useful if we have different asset packs (Kenney, Marketplace, etc).
/// </summary>
class PathRegistryService : public IService
{
public:
protected:
    void OnInitialize() override {} 
public: 
    void Clean() override { m_repos.clear(); }

    /// <summary>
    /// Registers a specific repository type.
    /// </summary>
    template <typename T>
    void RegisterRepository(std::shared_ptr<T> repo)
    {
        m_repos[std::type_index(typeid(T))] = repo;
        repo->Init(); 
    }

    /// <summary>
    /// Retrieves a registered repository by type.
    /// </summary>
    template <typename T>
    std::shared_ptr<T> GetRepository()
    {
        auto it = m_repos.find(std::type_index(typeid(T)));
        if (it == m_repos.end())
        {
            return nullptr;
        }
        return std::static_pointer_cast<T>(it->second);
    }

private:
    std::unordered_map<std::type_index, std::shared_ptr<IPathRepository>> m_repos;
};
