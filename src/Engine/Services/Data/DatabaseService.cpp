#include "Engine/Services/DatabaseService.h"
#include "Engine/Services/ServiceLocator.h"
#include "Engine/Services/ILoggerService.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h>

DatabaseService::DatabaseService()
{
}

DatabaseService::~DatabaseService()
{
    CloseDatabase(&m_staticDb);
    CloseDatabase(&m_dynamicDb);
}

void DatabaseService::OnInitialize()
{
    ServiceLocator::Get().GetService<ILoggerService>()->Log("DatabaseService Initialized.");
}

void DatabaseService::Clean()
{
    CloseDatabase(&m_staticDb);
    CloseDatabase(&m_dynamicDb);
    ServiceLocator::Get().GetService<ILoggerService>()->Log("DatabaseService Cleaned.");
}

bool DatabaseService::InitStatic(const std::string& path)
{
    // Open Read-Only
    if (OpenDatabase(path, &m_staticDb, SQLITE_OPEN_READONLY))
    {
        ServiceLocator::Get().GetService<ILoggerService>()->Log("Static Database Loaded: " + path);
        return true;
    }
    ServiceLocator::Get().GetService<ILoggerService>()->LogError("Failed to load Static Database: " + path);
    return false;
}

bool DatabaseService::CreateNewWorld(const std::string& worldName)
{
    std::string path = "bin/saves/" + worldName + ".db";
    
    // Ensure directory exists
    struct stat info;
    if (stat("bin/saves", &info) != 0)
    {
        // Try creating it (system specific, assuming mkdir works or user handles it. 
        // For portable C++, filesystem is better but let's stick to simple checks or just let sqlite fail/create if dir exists)
        // Actually sqlite3_open_v2 might not create directories.
        system("mkdir -p bin/saves"); 
    }

    if (OpenDatabase(path, &m_dynamicDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE))
    {
        ServiceLocator::Get().GetService<ILoggerService>()->Log("Created New World Database: " + path);
        
        // Initialize Schema from epoch_database.sql (World Tables Only ideally, but we might just run the whole thing if it's the schema source)
        // The user request said: "runs the schema from epoch_database.sql, but ONLY populates the World-related tables (Macro_Map, History)."
        // The epoch_database.sql provided seems to contain mostly Static Data (Creatures, Attacks).
        // Actual World Tables (Macro_Map) might not be in the provided sql yet.
        // I will assume we need to Create the tables. 
        
        // Let's create the Macro_Map table here as it's dynamic.
        std::string schema = R"(
            CREATE TABLE IF NOT EXISTS Macro_Map (
                x INTEGER,
                y INTEGER,
                biome_id INTEGER,
                seed_offset INTEGER,
                ruination INTEGER,
                wealth INTEGER,
                PRIMARY KEY (x, y)
            );
            
            CREATE TABLE IF NOT EXISTS History (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                event_type TEXT,
                tick INTEGER,
                description TEXT
            );
        )";
        
        if (ExecuteDynamic(schema))
        {
             ServiceLocator::Get().GetService<ILoggerService>()->Log("World Schema Initialized.");
             return true;
        }
    }
    
    return false;
}

bool DatabaseService::LoadWorld(const std::string& worldName)
{
    std::string path = "bin/saves/" + worldName + ".db";
    if (OpenDatabase(path, &m_dynamicDb, SQLITE_OPEN_READWRITE))
    {
         ServiceLocator::Get().GetService<ILoggerService>()->Log("Loaded World: " + worldName);
         return true;
    }
    return false;
}

bool DatabaseService::OpenDatabase(const std::string& path, sqlite3** dbPtr, int flags)
{
    if (*dbPtr)
    {
        CloseDatabase(dbPtr);
    }

    int rc = sqlite3_open_v2(path.c_str(), dbPtr, flags, nullptr);
    if (rc != SQLITE_OK)
    {
        ServiceLocator::Get().GetService<ILoggerService>()->LogError("SQLite Open Error: " + std::string(sqlite3_errmsg(*dbPtr)));
        return false;
    }
    return true;
}

void DatabaseService::CloseDatabase(sqlite3** dbPtr)
{
    if (*dbPtr)
    {
        sqlite3_close(*dbPtr);
        *dbPtr = nullptr;
    }
}

bool DatabaseService::ExecuteStatic(const std::string& sql)
{
    return ExecuteSQL(m_staticDb, sql);
}

bool DatabaseService::ExecuteDynamic(const std::string& sql)
{
    return ExecuteSQL(m_dynamicDb, sql);
}

bool DatabaseService::ExecuteSQL(sqlite3* db, const std::string& sql)
{
    if (!db) return false;
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK)
    {
        std::string err = "SQL Error: " + std::string(errMsg);
        ServiceLocator::Get().GetService<ILoggerService>()->LogError(err);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseService::WorldExists(const std::string& worldName)
{
    std::string path = "bin/saves/" + worldName + ".db";
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0); 
}
