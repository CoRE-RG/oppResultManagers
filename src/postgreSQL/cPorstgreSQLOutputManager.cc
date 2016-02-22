#include <cPorstgreSQLOutputManager.h>

Register_GlobalConfigOption(CFGID_POSTGRESQLOUTMGR_CONNECTION, "postgresqloutputmanager-connection", CFG_STRING, "\"\"",
        "Object name of database connection parameters");
Register_GlobalConfigOption(CFGID_POSTGRESQLOUTMGR_COMMIT_FREQ, "postgresqloutputmanager-commit-freq", CFG_INT, "10000",
        "COMMIT every n INSERTs, default=10");

#define SQL_SELECT_MODULE "SELECT * FROM module;"
#define SQL_INSERT_MODULE "INSERT INTO module(name) VALUES($1) RETURNING id"
#define SQL_SELECT_NAME "SELECT * FROM name;"
#define SQL_INSERT_NAME "INSERT INTO name(name) VALUES($1) RETURNING id"
#define SQL_INSERT_RUN "INSERT INTO run(runid,runnumber,network,date) VALUES($1,$2,$3,to_timestamp($4,'YYYYMMDD-HH24:MI:SS')) RETURNING id"
#define SQL_SELECT_RUN "SELECT id FROM run WHERE runid=$1;"

cPorstgreSQLOutputManager::cPorstgreSQLOutputManager()
{
    connection = nullptr;
    transaction = nullptr;
    runid = 0;
}

cPorstgreSQLOutputManager::~cPorstgreSQLOutputManager()
{
    if (transaction)
    {
        transaction->commit();
        delete transaction;
    }
    if (connection)
    {
        connection->disconnect();
        delete connection;
    }
}

void cPorstgreSQLOutputManager::startRun()
{
    if (!connection)
    {
        std::string cfgobj = ev.getConfig()->getAsString(CFGID_POSTGRESQLOUTMGR_CONNECTION);
        connection = new pqxx::connection(cfgobj.c_str());
    }

    commitFreq = ev.getConfig()->getAsInt(CFGID_POSTGRESQLOUTMGR_COMMIT_FREQ);

    pqxx::work work_transaction(*connection);
    //Create Tables
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS run (\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid TEXT NOT NULL UNIQUE,\
         runnumber BIGINT NOT NULL,\
         network TEXT NOT NULL,\
         date TIMESTAMP NOT NULL\
       );");
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS module(\
         id SERIAL NOT NULL PRIMARY KEY,\
         name TEXT NOT NULL UNIQUE\
       );");
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS name(\
         id SERIAL NOT NULL PRIMARY KEY,\
         name TEXT NOT NULL UNIQUE\
      );");

    //Find already existing run
    pqxx::result result = work_transaction.parameterized(SQL_SELECT_RUN)(ev.getConfigEx()->getVariable(CFGVAR_RUNID)).exec();
    if (result.size() > 1)
    {
        throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
    }
    else if (result.size() == 1)
    {
        runid = result[0][0].as<size_t>();
    }
    else
    {
        pqxx::result result = work_transaction.parameterized(SQL_INSERT_RUN)(
                ev.getConfigEx()->getVariable(CFGVAR_RUNID))(
        simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber())(
        simulation.getNetworkType()->getName())(ev.getConfigEx()->getVariable(CFGVAR_DATETIME)).exec();
        if (result.size() != 1)
        {
            throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
        }
        runid = result[0][0].as<size_t>();
    }

    //Find already existing modules and names
    result = work_transaction.exec(SQL_SELECT_MODULE);
    for (size_t rownum = 0; rownum < result.size(); ++rownum)
    {
        moduleIDMap[result[rownum][1].as<std::string>()] = result[rownum][0].as<size_t>();
    }
    result = work_transaction.exec(SQL_SELECT_NAME);
    for (size_t rownum = 0; rownum < result.size(); ++rownum)
    {
        nameIDMap[result[rownum][1].as<std::string>()] = result[rownum][0].as<size_t>();
    }
    work_transaction.commit();

    if (!transaction)
    {
        transaction = new pqxx::nontransaction(*connection);
        transaction->exec("BEGIN;");
    }
}

void cPorstgreSQLOutputManager::endRun()
{
    if (transaction)
    {
        transaction->exec("COMMIT;");
    }
}

void cPorstgreSQLOutputManager::flush()
{
    if (transaction)
    {
        transaction->exec("COMMIT; BEGIN;");
    }
}

size_t cPorstgreSQLOutputManager::getModuleID(std::string module)
{
    std::unordered_map<std::string, size_t>::const_iterator found = moduleIDMap.find(module);
    if (found != moduleIDMap.end())
    {
        return (*found).second;
    }
    else
    {

        pqxx::result result = transaction->parameterized(SQL_INSERT_MODULE)(module).exec();
        if (result.size() != 1)
        {
            //TODO check if now in database (concurrent runs possible!)
            throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
        }
        size_t id = result[0][0].as<size_t>();
        moduleIDMap[module] = id;
        transaction->exec("COMMIT; BEGIN;");
        return id;
    }
}

size_t cPorstgreSQLOutputManager::getNameID(std::string name)
{
    std::unordered_map<std::string, size_t>::const_iterator found = nameIDMap.find(name);
    if (found != nameIDMap.end())
    {
        return (*found).second;
    }
    else
    {
        pqxx::result result = transaction->parameterized(SQL_INSERT_NAME)(name).exec();
        if (result.size() != 1)
        {
            //TODO check if now in database (concurrent runs possible!)
            throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
        }
        size_t id = result[0][0].as<size_t>();
        nameIDMap[name] = id;
        transaction->exec("COMMIT; BEGIN;");
        return id;
    }
}
