#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "Engine/Services/UI/UIElement.h"

class BaseScreen {
public:
    BaseScreen(const std::string& id, const std::string&  guid) 
        : m_id(id), m_guid(guid) {}
    
    virtual ~BaseScreen() = default;

    virtual void OnLoad(std::shared_ptr<UIElement> root) {
        m_root = root;
        OnUnload(); 
        RegisterEvents();
    }
    
    virtual void OnShow() { m_isVisible = true; }
    virtual void OnHide() { m_isVisible = false; }
    virtual void OnUnload() { UnregisterAllEvents(); }
    
    std::shared_ptr<UIElement> FindElement(const std::string& id) {
        if (!m_root) return nullptr;
        if (m_root->id == id) return m_root;
        return m_root->FindChild(id);
    }

    std::string GetId() const { return m_id; }
    std::string GetGuid() const { return m_guid; }
    std::shared_ptr<UIElement> GetRoot() { return m_root; }

protected:
    virtual void RegisterEvents() {} // Default empty

    std::string m_id;
    std::string m_guid;
    std::shared_ptr<UIElement> m_root;
    bool m_isVisible = false;
    
    std::vector<std::function<void()>> m_eventCleanups;
    
    void UnregisterAllEvents() {
        for (auto& cleanup : m_eventCleanups) {
            cleanup();
        }
        m_eventCleanups.clear();
    }
};
