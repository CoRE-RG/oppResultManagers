#include "cSQLiteOutputScalarManager.h"

Register_Class(cSQLiteOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES(?,?,?,?);"
#define SQL_INSERT_SCALAR_ATTR "INSERT INTO scalarattr(scalarid,nameid,value) VALUES(?,?,?);"

void cSQLiteOutputScalarManager::startRun()
{
    cSQLiteOutputManager::startRun();
    char * zErrMsg = nullptr;
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalar (\
                     id INTEGER PRIMARY KEY,\
                     runid INT NOT NULL,\
                     moduleid INT NOT NULL,\
                     nameid INT NOT NULL,\
                     value DOUBLE PRECISION,\
                     FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
                     FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
                     FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                     CONSTRAINT scalar_unique UNIQUE (runid,moduleid,nameid)\
                   );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'scalar': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS scalar_names AS \
                             SELECT scalar.id AS id, scalar.runid AS runid, module.name AS module,\
                             name.name AS name, scalar.value AS value FROM scalar \
                             JOIN module ON module.id = scalar.moduleid \
                             JOIN name ON name.id = scalar.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create view 'scalar_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalarattr(\
                                     id INTEGER PRIMARY KEY,\
                                     scalarid INT NOT NULL,\
                                     nameid INT NOT NULL,\
                                     value TEXT NOT NULL,\
                                     FOREIGN KEY (scalarid) REFERENCES scalar(id) ON DELETE CASCADE,\
                                     FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
                                  );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'scalarattr': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS scalarattr_names AS \
                                     SELECT scalarattr.id AS id, scalarattr.scalarid AS scalarid, \
                                     name.name AS name, scalarattr.value AS value FROM scalarattr \
                                     JOIN name ON name.id = scalarattr.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create view 'scalarattr_names': %s", zErrMsg);
    }

    //prepare statements:
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_SCALAR_ATTR, strlen(SQL_INSERT_SCALAR_ATTR), &insertScalarAttrStmt,
            0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Could not prepare statement (SQL_INSERT_SCALAR_ATTR): %s",
                sqlite3_errmsg(connection));
    }
}

void cSQLiteOutputScalarManager::endRun()
{
    sqlite3_finalize(insertScalarAttrStmt);
    //TODO create index if parameter (TBD) is true
    cSQLiteOutputManager::endRun();
}

void cSQLiteOutputScalarManager::recordScalar(cComponent *component, const char *name, double value,
        opp_string_map *attributes)
{

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare(connection, SQL_INSERT_SCALAR_RESULT, strlen(SQL_INSERT_SCALAR_RESULT), &stmt, 0);
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
    if (isnan(value))
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
    size_t scalarId = static_cast<size_t>(sqlite3_last_insert_rowid(connection));
    sqlite3_reset(stmt);

    for (opp_string_map::iterator it = attributes->begin(); it != attributes->end(); ++it)
    {
        rc = sqlite3_bind_int64(insertScalarAttrStmt, 1, static_cast<sqlite3_int64>(scalarId));
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind vectorid.");
        }
        rc = sqlite3_bind_int64(insertScalarAttrStmt, 2, static_cast<sqlite3_int64>(getNameID(it->first.c_str())));
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind nameid.");
        }
        rc = sqlite3_bind_text(insertScalarAttrStmt, 3, it->second.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value.");
        }

        rc = sqlite3_step(insertScalarAttrStmt);
        if (rc != SQLITE_DONE)
        {
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR_ATTR): %s",
                    sqlite3_errmsg(connection));
        }
        sqlite3_clear_bindings(insertScalarAttrStmt);
        sqlite3_reset(insertScalarAttrStmt);
    }

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        flush();
    }
}

void cSQLiteOutputScalarManager::recordStatistic(__attribute__((__unused__))        cComponent *component,
        __attribute__((__unused__)) const char *name, __attribute__((__unused__))        cStatistic *statistic,
        __attribute__((__unused__))       opp_string_map *attributes)
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
