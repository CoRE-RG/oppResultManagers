#include "cSQLiteOutputScalarManager.h"

Register_Class(cSQLiteOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES(?,?,?,?);"

void cSQLiteOutputScalarManager::startRun()
{
    cSQLiteOutputManager::startRun();
    char * zErrMsg = nullptr;
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalar (\
                     runid INT NOT NULL,\
                     moduleid INT NOT NULL,\
                     nameid INT NOT NULL,\
                     value DOUBLE PRECISION,\
                     PRIMARY KEY (runid,moduleid,nameid),\
                     FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
                     FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
                     FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
                   );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'scalar': %s", zErrMsg);
    }
}

void cSQLiteOutputScalarManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    cSQLiteOutputManager::endRun();
}

void cSQLiteOutputScalarManager::recordScalar(cComponent *component, const char *name, double value,
        __attribute__((__unused__))  opp_string_map *attributes)
{

    sqlite3_stmt *stmt;
    size_t rc = sqlite3_prepare(connection, SQL_INSERT_SCALAR_RESULT, strlen(SQL_INSERT_SCALAR_RESULT), &stmt, 0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not prepare statement: %s", sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(runid));
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind active runnumber: %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(getModuleID(component->getFullPath())));
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind module id: %s", sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(getNameID(name)));
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind scalar name: %s", sqlite3_errmsg(connection));
    }
    if (isnanl(value))
    {
        sqlite3_bind_null(stmt, 4);
    }
    else
    {
        rc = sqlite3_bind_double(stmt, 4, value);
    }
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value: %s", sqlite3_errmsg(connection));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        throw cRuntimeError("cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_SCALAR_RESULT): %s",
                sqlite3_errmsg(connection));
    }
    sqlite3_reset(stmt);

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        flush();
    }
}

void cSQLiteOutputScalarManager::recordStatistic(__attribute__((__unused__))  cComponent *component,
        __attribute__((__unused__)) const char *name, __attribute__((__unused__))  cStatistic *statistic,
        __attribute__((__unused__)) opp_string_map *attributes)
{
    throw cRuntimeError("cPostgreSQLOutputScalarMgr: recording cStatistics objects not supported yet");
}

void cSQLiteOutputScalarManager::flush()
{
    cSQLiteOutputManager::flush();
}

const char *cSQLiteOutputScalarManager::getFileName() const
{
    return cSQLiteOutputManager::getFileName();
}
