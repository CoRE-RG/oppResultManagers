#include "cPostgreSQLOutputScalarManager.h"

Register_Class(cPostgreSQLOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES($1,$2,$3,$4) RETURNING id"
#define SQL_INSERT_SCALAR_ATTR "INSERT INTO scalarattr(scalarid,nameid,value) VALUES($1,$2,$3);"

void cPostgreSQLOutputScalarManager::startRun()
{
    cPorstgreSQLOutputManager::startRun();
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS scalar (\
             id SERIAL PRIMARY KEY,\
             runid INT NOT NULL,\
             moduleid INT NOT NULL,\
             nameid INT NOT NULL,\
             value DOUBLE PRECISION NOT NULL,\
             FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
             FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT scalar_unique UNIQUE (runid,moduleid,nameid)\
           );");
    transaction->exec(
            "CREATE OR REPLACE VIEW scalar_names AS \
             SELECT scalar.id AS id, scalar.runid AS runid, module.name AS module,\
             name.name AS name, scalar.value AS value FROM scalar \
             JOIN module ON module.id = scalar.moduleid \
             JOIN name ON name.id = scalar.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS scalarattr(\
             id SERIAL PRIMARY KEY,\
             scalarid INT NOT NULL,\
             nameid INT NOT NULL,\
             value TEXT NOT NULL,\
             FOREIGN KEY (scalarid) REFERENCES scalar(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
          );");
    transaction->exec(
            "CREATE OR REPLACE VIEW scalarattr_names AS \
             SELECT scalarattr.id AS id, scalarattr.scalarid AS scalarid, \
             name.name AS name, scalarattr.value AS value FROM scalarattr \
             JOIN name ON name.id = scalarattr.nameid;");
    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputScalarManager::endRun()
{
    //TODO create index if parameter (TBD) is true
    cPorstgreSQLOutputManager::endRun();
}

void cPostgreSQLOutputScalarManager::recordScalar(omnetpp::cComponent *component, const char *name, double value,
        omnetpp::opp_string_map *attributes)
{

    pqxx::result result = transaction->parameterized(SQL_INSERT_SCALAR_RESULT)(runid)(
            getModuleID(component->getFullPath()))(getNameID(name))(value).exec();
    if (result.size() != 1)
    {
        throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarManager:: internal error!");
    }
    size_t vectorId = result[0][0].as<size_t>();

    for (omnetpp::opp_string_map::const_iterator it = attributes->begin(); it != attributes->end(); ++it)
    {
        transaction->parameterized(SQL_INSERT_SCALAR_ATTR)(vectorId)(getNameID(it->first.c_str()))(it->second.c_str()).exec();
    }

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        if (transaction)
        {
            transaction->exec("COMMIT; BEGIN;");
        }
    }
}

void cPostgreSQLOutputScalarManager::recordStatistic(__attribute__((__unused__)) omnetpp::cComponent *component,
        __attribute__((__unused__)) const char *name, __attribute__((__unused__)) omnetpp::cStatistic *statistic,
        __attribute__((__unused__)) omnetpp::opp_string_map *attributes)
{
    throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr: recording cStatistics objects not supported yet");
}

void cPostgreSQLOutputScalarManager::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputScalarManager::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
