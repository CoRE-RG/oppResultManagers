#include "cPostgreSQLOutputScalarManager.h"

Register_Class(cPostgreSQLOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES($1,$2,$3,$4)"

void cPostgreSQLOutputScalarManager::startRun()
{
    cPorstgreSQLOutputManager::startRun();
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS scalar (\
             runid INT NOT NULL,\
             moduleid INT NOT NULL,\
             nameid INT NOT NULL,\
             value DOUBLE PRECISION NOT NULL,\
             PRIMARY KEY (runid,moduleid,nameid),\
             FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
             FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
           );");
    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputScalarManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    cPorstgreSQLOutputManager::endRun();
}

void cPostgreSQLOutputScalarManager::recordScalar(cComponent *component, const char *name, double value,
        __attribute__((__unused__))  opp_string_map *attributes)
{

    transaction->parameterized(SQL_INSERT_SCALAR_RESULT)(runid)(getModuleID(component->getFullPath()))(getNameID(name))(
            value).exec();

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        if (transaction)
        {
            transaction->exec("COMMIT; BEGIN;");
        }
    }
}

void cPostgreSQLOutputScalarManager::recordStatistic(__attribute__((__unused__))  cComponent *component,
        __attribute__((__unused__)) const char *name, __attribute__((__unused__))  cStatistic *statistic,
        __attribute__((__unused__))  opp_string_map *attributes)
{
    throw cRuntimeError("cPostgreSQLOutputScalarMgr: recording cStatistics objects not supported yet");
}

void cPostgreSQLOutputScalarManager::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputScalarManager::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
