#include "cSQLiteOutputScalarMgr.h"

Register_Class(cSQLiteOutputScalarMgr);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES(?,?,?,?)"

void cSQLiteOutputScalarMgr::startRun()
{
    cSQLiteOutputManager::startRun();
    char * zErrMsg = nullptr;
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalar (\
                     runid INT NOT NULL,\
                     moduleid INT NOT NULL,\
                     nameid INT NOT NULL,\
                     value DOUBLE PRECISION NOT NULL,\
                     PRIMARY KEY (runid,moduleid,nameid),\
                     FOREIGN KEY (runid) REFERENCES run(id),\
                     FOREIGN KEY (moduleid) REFERENCES module(id),\
                     FOREIGN KEY (nameid) REFERENCES name(id)\
                   );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'scalar': %s", zErrMsg);
    }
}

void cSQLiteOutputScalarMgr::endRun()
{
    //TODO create index if parameter (TBD) is true
    cSQLiteOutputManager::endRun();
}

void cSQLiteOutputScalarMgr::recordScalar(cComponent *component, const char *name, double value,
        opp_string_map *attributes)
{

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare(connection, SQL_INSERT_SCALAR_RESULT, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not prepare statement.");
    }
    rc = sqlite3_bind_int(stmt, 1, runid);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not bind active runnumber.");
    }
    rc = sqlite3_bind_int(stmt, 2, getModuleID(component->getFullPath()));
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not bind active runnumber.");
    }
    rc = sqlite3_bind_int(stmt, 3, getNameID(name));
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not bind active runnumber.");
    }
    rc = sqlite3_bind_double(stmt, 4, value);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not bind active runnumber.");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_close(connection);
        throw cRuntimeError("SQLiteOutputManager:: Could not execute statement.");
    }
    sqlite3_reset(stmt);

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        flush();
    }
}

void cSQLiteOutputScalarMgr::recordStatistic(cComponent *component, const char *name, cStatistic *statistic,
        opp_string_map *attributes)
{
    throw cRuntimeError("cPostgreSQLOutputScalarMgr: recording cStatistics objects not supported yet");
}

void cSQLiteOutputScalarMgr::flush()
{
    cSQLiteOutputManager::flush();
}

const char *cSQLiteOutputScalarMgr::getFileName() const
{
    return cSQLiteOutputManager::getFileName();
}
