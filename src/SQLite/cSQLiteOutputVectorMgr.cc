#include "cSQLiteOutputVectorMgr.h"

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
                             FOREIGN KEY (runid) REFERENCES run(id),\
                             FOREIGN KEY (moduleid) REFERENCES module(id),\
                             FOREIGN KEY (nameid) REFERENCES name(id)\
                          );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vector': %s", zErrMsg);
    }

    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS vectorattr(\
                                 id INTEGER PRIMARY KEY,\
                                 vectorid INT NOT NULL,\
                                 nameid INT NOT NULL,\
                                 value TEXT NOT NULL,\
                                 FOREIGN KEY (vectorid) REFERENCES vector(id),\
                                 FOREIGN KEY (nameid) REFERENCES name(id)\
                              );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vectorattr': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS vectordata (\
                                         vectorid INT NOT NULL,\
                                         time DOUBLE PRECISION NOT NULL,\
                                         value DOUBLE PRECISION NOT NULL,\
                                         FOREIGN KEY (vectorid) REFERENCES vector(id)\
                                       );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputScalarMgr:: Can't create table 'vectordata': %s", zErrMsg);
    }
}

void cSQLiteOutputVectorManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    cSQLiteOutputManager::endRun();
}

void *cSQLiteOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;
    vp->enabled = true;
    return vp;
}
void cSQLiteOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *) vectorhandle;
    delete vp;
}
void cSQLiteOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    ASSERT(vectorhandle != NULL);
    sVectorData *vp = (sVectorData *) vectorhandle;
    vp->attributes[name] = value;
}
bool cSQLiteOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *) vectorhandle;

    if (!vp->enabled)
        return false;

    sqlite3_stmt *stmt;
    if (!vp->initialised)
    {
        int rc = sqlite3_prepare(connection, SQL_INSERT_VECTOR, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            sqlite3_close(connection);
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement.");
        }
        rc = sqlite3_bind_int(stmt, 1, runid);
        if (rc != SQLITE_OK)
        {
            sqlite3_close(connection);
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active runid.");
        }
        rc = sqlite3_bind_int(stmt, 2, getModuleID(vp->modulename.c_str()));
        if (rc != SQLITE_OK)
        {
            sqlite3_close(connection);
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active moduleid.");
        }
        rc = sqlite3_bind_int(stmt, 3, getNameID(vp->vectorname.c_str()));
        if (rc != SQLITE_OK)
        {
            sqlite3_close(connection);
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind active nameid.");
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            sqlite3_close(connection);
            throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR).");
        }
        vp->id = sqlite3_last_insert_rowid(connection);
        sqlite3_reset(stmt);
        vp->initialised = true;
        for (opp_string_map::iterator it = vp->attributes.begin(); it != vp->attributes.end(); ++it)
        {
            int rc = sqlite3_prepare(connection, SQL_INSERT_VECTOR_ATTR, -1, &stmt, 0);
            if (rc != SQLITE_OK)
            {
                sqlite3_close(connection);
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement.");
            }
            rc = sqlite3_bind_int(stmt, 1, vp->id);
            if (rc != SQLITE_OK)
            {
                sqlite3_close(connection);
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind vectorid.");
            }
            rc = sqlite3_bind_int(stmt, 2, getNameID(it->first.c_str()));
            if (rc != SQLITE_OK)
            {
                sqlite3_close(connection);
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind nameid.");
            }
            rc = sqlite3_bind_text(stmt, 3, it->second.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                sqlite3_close(connection);
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind value.");
            }

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                sqlite3_close(connection);
                throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR_ATTR).");
            }
            vp->id = sqlite3_last_insert_rowid(connection);
            sqlite3_reset(stmt);
        }
    }

    // fill in prepared statement parameters, and fire off the statement
    int rc = sqlite3_prepare(connection, SQL_INSERT_VECTOR_DATA, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not prepare statement.");
    }
    rc = sqlite3_bind_int(stmt, 1, vp->id);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind vectorid.");
    }
    rc = sqlite3_bind_double(stmt, 2, SIMTIME_DBL(t));
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind time.");
    }
    rc = sqlite3_bind_double(stmt, 3, value);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not bind value.");
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_close(connection);
        throw cRuntimeError("cSQLiteOutputVectorManager:: Could not execute statement (SQL_INSERT_VECTOR_DATA).");
    }
    vp->id = sqlite3_last_insert_rowid(connection);
    sqlite3_reset (stmt);

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        flush();
    }
    return true;
}

void cSQLiteOutputVectorManager::flush()
{
    cSQLiteOutputManager::flush();
}

const char *cSQLiteOutputVectorManager::getFileName() const
{
    return cSQLiteOutputManager::getFileName();
}
