#include "cSQLiteOutputVectorManager.h"

#include "HelperFunctions.h"

Register_Class(cSQLiteOutputVectorManager);

#define SQL_INSERT_VECTOR "INSERT INTO vector(runid, moduleid, nameid) VALUES(?,?,?);"
#define SQL_INSERT_VECTOR_ATTR "INSERT INTO vectorattr(vectorid,nameid,value) VALUES(?,?,?);"
#define SQL_INSERT_VECTOR_DATA  "INSERT INTO vectordata(vectorid,time,value) VALUES(?,?,?);"

void cSQLiteOutputVectorManager::startRun()
{
    cSQLiteOutputManager::startRun();

    //Create Tables
    char * zErrMsg = nullptr;
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS vector(\
                             id INTEGER PRIMARY KEY,\
                             runid INT NOT NULL,\
                             moduleid INT NOT NULL,\
                             nameid INT NOT NULL,\
                             FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
                             FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
                             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                             CONSTRAINT vector_unique UNIQUE(runid, moduleid, nameid) \
                          );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vector': %s", zErrMsg);
    }

    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS vectorattr(\
                                 id INTEGER PRIMARY KEY,\
                                 vectorid INT NOT NULL,\
                                 nameid INT NOT NULL,\
                                 value TEXT NOT NULL,\
                                 FOREIGN KEY (vectorid) REFERENCES vector(id) ON DELETE CASCADE,\
                                 FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                                 CONSTRAINT vectorattr_unique UNIQUE(vectorid, nameid, value) \
                              );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vectorattr': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS vectordata (\
                                         vectorid INT NOT NULL,\
                                         time DOUBLE PRECISION NOT NULL,\
                                         value DOUBLE PRECISION NOT NULL,\
                                         FOREIGN KEY (vectorid) REFERENCES vector(id) ON DELETE CASCADE,\
                                         CONSTRAINT vectordata_unique UNIQUE(vectorid, time, value) \
                                       );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vectordata': %s", zErrMsg);
    }

    //prepare statements:
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_VECTOR, strlen(SQL_INSERT_VECTOR), &insertVectorStmt, 0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement (SQL_INSERT_VECTOR_ATTR): %s",
                sqlite3_errmsg(connection));
    }

    rc = sqlite3_prepare_v2(connection, SQL_INSERT_VECTOR_ATTR, strlen(SQL_INSERT_VECTOR_ATTR), &insertVectorAttrStmt,
            0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement (SQL_INSERT_VECTOR_ATTR): %s",
                sqlite3_errmsg(connection));
    }

    rc = sqlite3_prepare_v2(connection, SQL_INSERT_VECTOR_DATA, strlen(SQL_INSERT_VECTOR_DATA), &insertVectorDataStmt,
            0);
    if (rc != SQLITE_OK)
    {
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement (SQL_INSERT_VECTOR_DATA): %s",
                sqlite3_errmsg(connection));
    }
}

void cSQLiteOutputVectorManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    sqlite3_finalize(insertVectorStmt);
    sqlite3_finalize(insertVectorAttrStmt);
    sqlite3_finalize(insertVectorDataStmt);
    cSQLiteOutputManager::endRun();
}

bool cSQLiteOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *) vectorhandle;

    if (vp->enabled && inIntervals(t, vp->intervals))
    {
        if (!vp->initialised)
        {
            int rc = sqlite3_bind_int(insertVectorStmt, 1, runid);
            if (rc != SQLITE_OK)
            {
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active runid.");
            }
            rc = sqlite3_bind_int(insertVectorStmt, 2, getModuleID(vp->modulename.c_str()));
            if (rc != SQLITE_OK)
            {
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active moduleid.");
            }
            rc = sqlite3_bind_int(insertVectorStmt, 3, getNameID(vp->vectorname.c_str()));
            if (rc != SQLITE_OK)
            {
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active nameid.");
            }

            rc = sqlite3_step(insertVectorStmt);
            if (rc != SQLITE_DONE)
            {
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR): %s",
                        sqlite3_errmsg(connection));
            }
            vp->id = sqlite3_last_insert_rowid(connection);
            sqlite3_clear_bindings(insertVectorStmt);
            sqlite3_reset(insertVectorStmt);
            vp->initialised = true;
            for (opp_string_map::iterator it = vp->attributes.begin(); it != vp->attributes.end(); ++it)
            {
                rc = sqlite3_bind_int(insertVectorAttrStmt, 1, vp->id);
                if (rc != SQLITE_OK)
                {
                    throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind vectorid.");
                }
                int rc = sqlite3_bind_int(insertVectorAttrStmt, 2, getNameID(it->first.c_str()));
                if (rc != SQLITE_OK)
                {
                    throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind nameid.");
                }
                rc = sqlite3_bind_text(insertVectorAttrStmt, 3, it->second.c_str(), -1, SQLITE_STATIC);
                if (rc != SQLITE_OK)
                {
                    throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind value.");
                }

                rc = sqlite3_step(insertVectorAttrStmt);
                if (rc != SQLITE_DONE)
                {
                    throw cRuntimeError(
                            "cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR_ATTR): %s",
                            sqlite3_errmsg(connection));
                }
                sqlite3_clear_bindings(insertVectorAttrStmt);
                sqlite3_reset(insertVectorAttrStmt);
            }
        }

        // fill in prepared statement parameters, and fire off the statement
        int rc = sqlite3_bind_int(insertVectorDataStmt, 1, vp->id);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind vectorid.");
        }
        rc = sqlite3_bind_double(insertVectorDataStmt, 2, SIMTIME_DBL(t));
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind time.");
        }
        rc = sqlite3_bind_double(insertVectorDataStmt, 3, value);
        if (rc != SQLITE_OK)
        {
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind value.");
        }

        rc = sqlite3_step(insertVectorDataStmt);
        if (rc != SQLITE_DONE)
        {
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR_DATA): %s",
                    sqlite3_errmsg(connection));
        }
        sqlite3_clear_bindings(insertVectorDataStmt);
        sqlite3_reset(insertVectorDataStmt);

        // commit every once in a while
        if (commitFreq && ((++insertCount % commitFreq) == 0))
        {
            flush();
        }
        return true;
    }
    else
    {
        return false;
    }
}

void cSQLiteOutputVectorManager::flush()
{
    cSQLiteOutputManager::flush();
}

const char *cSQLiteOutputVectorManager::getFileName() const
{
    return cSQLiteOutputManager::getFileName();
}
