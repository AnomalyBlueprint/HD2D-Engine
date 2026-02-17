#pragma once
#include "Engine/Services/IService.h"
#include <string>
#include <vector>
#include <sqlite3.h>

class DatabaseService : public IService
{
public:
    DatabaseService();
    ~DatabaseService();

    // IService
    void OnInitialize() override;
    void Clean() override;

    // Database Management
    bool InitStatic(const std::string& path);
    bool CreateNewWorld(const std::string& worldName);
    bool LoadWorld(const std::string& worldName);

    // SQL Execution
    bool ExecuteStatic(const std::string& sql);
    bool ExecuteDynamic(const std::string& sql);
    
    // Helper to check if world exists
    bool WorldExists(const std::string& worldName);

private:
    sqlite3* m_staticDb = nullptr;
    sqlite3* m_dynamicDb = nullptr;

    bool OpenDatabase(const std::string& path, sqlite3** dbPtr, int flags);
    void CloseDatabase(sqlite3** dbPtr);
    bool ExecuteSQL(sqlite3* db, const std::string& sql);
};
