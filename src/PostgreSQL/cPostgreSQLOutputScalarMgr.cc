#include "cPostgreSQLOutputScalarMgr.h"

Register_Class(cPostgreSQLOutputScalarMgr);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES($1,$2,$3,$4)"

void cPostgreSQLOutputScalarMgr::startRun()
{
    cPorstgreSQLOutputManager::startRun();
    transaction->exec(
                "CREATE TABLE IF NOT EXISTS scalar (\
             runid INT NOT NULL,\
             moduleid INT NOT NULL,\
             nameid INT NOT NULL,\
             value DOUBLE PRECISION NOT NULL,\
             PRIMARY KEY (runid,moduleid,nameid),\
             FOREIGN KEY (runid) REFERENCES run(id),\
             FOREIGN KEY (moduleid) REFERENCES module(id),\
             FOREIGN KEY (nameid) REFERENCES name(id)\
           );");
    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputScalarMgr::endRun()
{
    cPorstgreSQLOutputManager::endRun();
}

void cPostgreSQLOutputScalarMgr::recordScalar(cComponent *component, const char *name, double value,
        opp_string_map *attributes)
{

    transaction->parameterized(SQL_INSERT_SCALAR_RESULT)(runid)(getModuleID(component->getFullPath()))(getNameID(name))(
            value).exec();

    // commit every once in a while
    if ((++insertCount % commitFreq) == 0)
    {
        if (transaction)
        {
            transaction->exec("COMMIT; BEGIN;");
        }
    }
}

void cPostgreSQLOutputScalarMgr::recordStatistic(cComponent *component, const char *name, cStatistic *statistic,
        opp_string_map *attributes)
{
    throw cRuntimeError("cPostgreSQLOutputScalarMgr: recording cStatistics objects not supported yet");
}

void cPostgreSQLOutputScalarMgr::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputScalarMgr::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
