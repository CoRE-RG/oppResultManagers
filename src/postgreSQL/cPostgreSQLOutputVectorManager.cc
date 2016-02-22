#include "cPostgreSQLOutputVectorManager.h"

#include "HelperFunctions.h"

Register_Class(cPostgreSQLOutputVectorManager);

#define SQL_INSERT_VECTOR "INSERT INTO vector(runid, moduleid, nameid) VALUES($1,$2,$3) RETURNING id"
#define SQL_INSERT_VECTOR_ATTR "INSERT INTO vectorattr(vectorid,nameid,value) VALUES($1,$2,$3)"
#define SQL_INSERT_VECTOR_DATA  "INSERT INTO vectordata(vectorid,time,value) VALUES($1,$2,$3)"

void cPostgreSQLOutputVectorManager::startRun()
{
    cPorstgreSQLOutputManager::startRun();

    //Create Tables
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vector(\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid INT NOT NULL,\
         moduleid INT NOT NULL,\
         nameid INT NOT NULL,\
         FOREIGN KEY (runid) REFERENCES run(id),\
         FOREIGN KEY (moduleid) REFERENCES module(id),\
         FOREIGN KEY (nameid) REFERENCES name(id)\
      );");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vectorattr(\
         id SERIAL NOT NULL PRIMARY KEY,\
         vectorid INT NOT NULL,\
         nameid INT NOT NULL,\
         value TEXT NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id),\
         FOREIGN KEY (nameid) REFERENCES name(id)\
      );");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vectordata (\
         vectorid INT NOT NULL,\
         time DOUBLE PRECISION NOT NULL,\
         value DOUBLE PRECISION NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id)\
       );");
    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputVectorManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    cPorstgreSQLOutputManager::endRun();
}

bool cPostgreSQLOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *) vectorhandle;

    if (vp->enabled && inIntervals(t, vp->intervals))
    {
        if (!vp->initialised)
        {
            pqxx::result result = transaction->parameterized(SQL_INSERT_VECTOR)(runid)(
                    getModuleID(vp->modulename.c_str()))(getNameID(vp->vectorname.c_str())).exec();
            if (result.size() != 1)
            {
                throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            vp->id = result[0][0].as<long>();
            vp->initialised = true;

            for (opp_string_map::iterator it = vp->attributes.begin(); it != vp->attributes.end(); ++it)
            {
                transaction->parameterized(SQL_INSERT_VECTOR_ATTR)(vp->id)(getNameID(it->first.c_str()))(
                        it->second.c_str()).exec();
            }
        }

        // fill in prepared statement parameters, and fire off the statement
        transaction->parameterized(SQL_INSERT_VECTOR_DATA)(vp->id)(SIMTIME_DBL(t))(value).exec();

        // commit every once in a while
        if (commitFreq && ((++insertCount % commitFreq) == 0))
        {
            if (transaction)
            {
                transaction->exec("COMMIT; BEGIN;");
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void cPostgreSQLOutputVectorManager::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputVectorManager::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
