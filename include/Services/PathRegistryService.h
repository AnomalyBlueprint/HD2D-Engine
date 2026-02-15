#pragma once
#include "Services/IService.h"
#include "Services/IPathRepository.h"
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>

class PathRegistryService : public IService
{
public:
    void Init() override {} // Nothing specific to init
    void Clean() override { m_repos.clear(); }

    template <typename T>
    void RegisterRepository(std::shared_ptr<T> repo)
    {
        // static_assert(std::is_base_of<IPathRepository, T>::value, "Must implement IPathRepository");
        m_repos[std::type_index(typeid(T))] = repo;
        repo->Init(); 
    }

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
