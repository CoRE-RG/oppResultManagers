#include <cSQLiteOutputManager.h>

Register_GlobalConfigOption(CFGID_SQLITEOUTMGR_FILE, "sqliteoutputmanager-file", CFG_STRING,
        "${resultdir}/${configname}-${runnumber}.sqlite3", "Object name of database connection parameters");
Register_GlobalConfigOption(CFGID_SQLITEMGR_COMMIT_FREQ, "sqliteoutputmanager-commit-freq", CFG_INT, "10000",
        "COMMIT every n INSERTs, default=10");

#define SQL_SELECT_MODULE "SELECT * FROM module;"
#define SQL_INSERT_MODULE "INSERT INTO module(name) VALUES(?);"
#define SQL_SELECT_NAME "SELECT * FROM name;"
#define SQL_INSERT_NAME "INSERT INTO name(name) VALUES(?);"
#define SQL_INSERT_RUN "INSERT INTO run(runnumber,network) VALUES(?,?);"

sqlite3* cSQLiteOutputManager::connection = nullptr;
bool cSQLiteOutputManager::hasTransaction = false;
size_t cSQLiteOutputManager::users = 0;

std::unordered_map<std::string, size_t> cSQLiteOutputManager::moduleIDMap;
std::unordered_map<std::string, size_t> cSQLiteOutputManager::nameIDMap;

cSQLiteOutputManager::cSQLiteOutputManager()
{
    runid = 0;
    users++;
}

cSQLiteOutputManager::~cSQLiteOutputManager()
{
    users--;
    if (connection && users == 0)
    {
        sqlite3_close(connection);
    }
}

void cSQLiteOutputManager::startRun()
{
    if (!connection)
    {
        std::string cfgobj = ev.getConfig()->getAsString(CFGID_SQLITEOUTMGR_FILE);
        int rc = sqlite3_open(cfgobj.c_str(), &connection);
        if (rc)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't open database: %s", sqlite3_errmsg(connection));
        }
        char * zErrMsg = nullptr;
        rc = sqlite3_exec(connection, "PRAGMA synchronous = OFF;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't set PRAGMA synchronous = OFF: %s", zErrMsg);
        }
        rc = sqlite3_exec(connection, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't set PRAGMA journal_mode = MEMORY: %s", zErrMsg);
        }
        rc = sqlite3_exec(connection, "PRAGMA cache_size = -16768;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't set PRAGMA cache_size: %s", zErrMsg);
        }
    }

    commitFreq = ev.getConfig()->getAsInt(CFGID_SQLITEMGR_COMMIT_FREQ);

    char * zErrMsg = nullptr;

    if (!hasTransaction)
    {
        int rc = sqlite3_exec(connection, "BEGIN;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't begin transaction: %s", zErrMsg);
        }
        hasTransaction = true;
    }

    //Create Tables
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS run (\
         id INTEGER PRIMARY KEY,\
         runnumber BIGINT NOT NULL,\
         network TEXT NOT NULL,\
         date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP\
       );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Can't create table 'run': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS module(\
                 id INTEGER PRIMARY KEY,\
                 name TEXT NOT NULL UNIQUE\
               );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Can't create table 'module': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS name(\
                 id INTEGER PRIMARY KEY,\
                 name TEXT NOT NULL UNIQUE\
              );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Can't create table 'name': %s", zErrMsg);
    }

    flush();

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare(connection, SQL_INSERT_RUN, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Could not prepare statement: %s", sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int(stmt, 1, simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber());
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Could not bind active runnumber: %s", sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_text(stmt, 2, simulation.getNetworkType()->getName(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Could not bind network name.");
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        throw cRuntimeError("SQLiteOutputManager:: Could not execute statement (SQL_INSERT_RUN): %s",
                sqlite3_errmsg(connection));
    }
    runid = sqlite3_last_insert_rowid(connection);
    sqlite3_reset(stmt);

    //Find already existing modules and names
    rc = sqlite3_exec(connection, SQL_SELECT_MODULE, [] (void *data, int argc, char **argv, char **azColName) -> int
    {
        cSQLiteOutputManager *thisManager = (cSQLiteOutputManager*)data;
        if(argc!=2)
        {
            throw cRuntimeError("wrong number of columns returned in select!");
        }
        thisManager->moduleIDMap[argv[1]] = atoi(argv[0]);
        return SQLITE_OK;
    }, (void*) this, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Error in select (SQL_SELECT_MODULE): %s", zErrMsg);
    }

    rc = sqlite3_exec(connection, SQL_SELECT_NAME, [] (void *data, int argc, char **argv, char **azColName) -> int
    {
        cSQLiteOutputManager *thisManager = (cSQLiteOutputManager*)data;
        if(argc!=2)
        {
            throw cRuntimeError("wrong number of columns returned in select!");
        }
        thisManager->nameIDMap[argv[1]] = atoi(argv[0]);
        return SQLITE_OK;
    }, (void*) this, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("SQLiteOutputManager:: Error in select (SQL_SELECT_NAME): %s", zErrMsg);
    }

}

void cSQLiteOutputManager::endRun()
{
    if (connection && hasTransaction)
    {
        char * zErrMsg = nullptr;
        int rc = sqlite3_exec(connection, "COMMIT;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't commit: %s", zErrMsg);
        }
        hasTransaction = false;
    }
}

void cSQLiteOutputManager::flush()
{
    if (connection)
    {
        char * zErrMsg = nullptr;
        int rc = sqlite3_exec(connection, "COMMIT; BEGIN;", nullptr, nullptr, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Can't commit: %s", zErrMsg);
        }
    }
}

size_t cSQLiteOutputManager::getModuleID(std::string module)
{
    std::unordered_map<std::string, size_t>::const_iterator found = moduleIDMap.find(module);
    if (found != moduleIDMap.end())
    {
        return (*found).second;
    }
    else
    {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare(connection, SQL_INSERT_MODULE, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not prepare statement: %s", sqlite3_errmsg(connection));
        }
        rc = sqlite3_bind_text(stmt, 1, module.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not bind module: %s", sqlite3_errmsg(connection));
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not execute statement (SQL_INSERT_MODULE): %s",
                    sqlite3_errmsg(connection));
        }
        size_t id = sqlite3_last_insert_rowid(connection);
        moduleIDMap[module] = id;
        sqlite3_reset(stmt);

        flush();
        return id;
    }
}

size_t cSQLiteOutputManager::getNameID(std::string name)
{
    std::unordered_map<std::string, size_t>::const_iterator found = nameIDMap.find(name);
    if (found != nameIDMap.end())
    {
        return (*found).second;
    }
    else
    {
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare(connection, SQL_INSERT_NAME, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not prepare statement: %s", sqlite3_errmsg(connection));
        }
        rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not bind name: %s", sqlite3_errmsg(connection));
        }
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            throw cRuntimeError("SQLiteOutputManager:: Could not execute statement (SQL_INSERT_NAME): %s",
                    sqlite3_errmsg(connection));
        }
        size_t id = sqlite3_last_insert_rowid(connection);
        nameIDMap[name] = id;
        sqlite3_reset(stmt);

        flush();
        return id;
    }
}
