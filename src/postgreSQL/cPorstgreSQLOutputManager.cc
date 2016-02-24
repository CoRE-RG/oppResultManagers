#include <cPorstgreSQLOutputManager.h>

Register_PerRunConfigOption(CFGID_POSTGRESQLOUTMGR_CONNECTION, "postgresqloutputmanager-connection", CFG_STRING, "\"\"",
        "Object name of database connection parameters");
Register_PerRunConfigOption(CFGID_POSTGRESQLOUTMGR_COMMIT_FREQ, "postgresqloutputmanager-commit-freq", CFG_INT, "10000",
        "COMMIT every n INSERTs, default=10");

#define SQL_SELECT_MODULE "SELECT * FROM module;"
#define SQL_SELECT_MODULE_BYNAME  "SELECT id FROM module WHERE name=$1;"
#define SQL_INSERT_MODULE "INSERT INTO module(name) VALUES($1) RETURNING id"
#define SQL_SELECT_NAME "SELECT * FROM name;"
#define SQL_SELECT_NAME_BYNAME  "SELECT id FROM name WHERE name=$1;"
#define SQL_INSERT_NAME "INSERT INTO name(name) VALUES($1) RETURNING id"
#define SQL_INSERT_RUN "INSERT INTO run(runid) VALUES($1) RETURNING id"
#define SQL_SELECT_RUN "SELECT id FROM run WHERE runid=$1;"
#define SQL_INSERT_RUN_ATTR "INSERT INTO runattr(runid,nameid,value) VALUES($1,$2,$3);"

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
        std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_POSTGRESQLOUTMGR_CONNECTION);
        connection = new pqxx::connection(cfgobj.c_str());
    }

    commitFreq = omnetpp::getEnvir()->getConfig()->getAsInt(CFGID_POSTGRESQLOUTMGR_COMMIT_FREQ);

    pqxx::work work_transaction(*connection);
    //Create Tables
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS run (\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid TEXT NOT NULL UNIQUE\
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
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS runattr(\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid INT NOT NULL,\
         nameid INT NOT NULL,\
         value TEXT NOT NULL,\
         FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
         FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
       );");
    work_transaction.exec(
            "CREATE OR REPLACE VIEW runattr_names AS \
         SELECT runattr.id AS id, runattr.runid AS runid, \
         name.name AS name, runattr.value AS value FROM runattr \
         JOIN name ON name.id = runattr.nameid;");

    work_transaction.commit();
    if (!transaction)
    {
        transaction = new pqxx::nontransaction(*connection);
        transaction->exec("BEGIN;");
    }
    //Find already existing modules and names
    pqxx::result result = transaction->exec(SQL_SELECT_MODULE);
    for (size_t rownum = 0; rownum < result.size(); ++rownum)
    {
        moduleIDMap[result[rownum][1].as<std::string>()] = result[rownum][0].as<size_t>();
    }
    result = transaction->exec(SQL_SELECT_NAME);
    for (size_t rownum = 0; rownum < result.size(); ++rownum)
    {
        nameIDMap[result[rownum][1].as<std::string>()] = result[rownum][0].as<size_t>();
    }

    //Find already existing run
    result =
            transaction->parameterized(SQL_SELECT_RUN)
            (omnetpp::getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID)).exec();
    if (result.size() > 1)
    {
        throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
    }
    else if (result.size() == 1)
    {
        runid = result[0][0].as<size_t>();
    }
    else
    {
        result = transaction->parameterized(SQL_INSERT_RUN)
                (omnetpp::getEnvir()->getConfigEx()->getVariable(CFGVAR_RUNID)).exec();
        if (result.size() != 1)
        {
            throw omnetpp::cRuntimeError("cPostgreSQLOutputMgr:: internal error!");
        }
        runid = result[0][0].as<size_t>();

        //INSERT runattr
        std::vector<const char *> keys1 = omnetpp::getEnvir()->getConfigEx()->getPredefinedVariableNames();
        std::vector<const char *> keys2 = omnetpp::getEnvir()->getConfigEx()->getIterationVariableNames();
        keys1.insert(keys1.end(), keys2.begin(), keys2.end());
        for (size_t i = 0; i < keys1.size(); i++)
        {
            work_transaction.parameterized(SQL_INSERT_RUN_ATTR)(runid)(getNameID(keys1[i]))(
                    omnetpp::getEnvir()->getConfigEx()->getVariable(keys1[i])).exec();
        }
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
        try
        {
            pqxx::result result = transaction->parameterized(SQL_INSERT_MODULE)(module).exec();
            if (result.size() != 1)
            {
                throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            size_t id = result[0][0].as<size_t>();
            moduleIDMap[module] = id;
            transaction->exec("COMMIT; BEGIN;");
            return id;
        }
        catch (pqxx::unique_violation e)
        {
            transaction->exec("COMMIT; BEGIN;");
            pqxx::result result = transaction->parameterized(SQL_SELECT_MODULE_BYNAME)(module).exec();
            if (result.size() != 1)
            {
                throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            else
            {
                size_t id = result[0][0].as<size_t>();
                moduleIDMap[module] = id;
                return id;
            }
        }
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
        try
        {
            pqxx::result result = transaction->parameterized(SQL_INSERT_NAME)(name).exec();
            if (result.size() != 1)
            {
                //TODO check if now in database (concurrent runs possible!)

                throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            size_t id = result[0][0].as<size_t>();
            nameIDMap[name] = id;
            transaction->exec("COMMIT; BEGIN;");
            return id;
        }
        catch (pqxx::unique_violation e)
        {
            transaction->exec("COMMIT; BEGIN;");
            pqxx::result result = transaction->parameterized(SQL_SELECT_NAME_BYNAME)(name).exec();
            if (result.size() != 1)
            {
                throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            else
            {
                size_t id = result[0][0].as<size_t>();
                nameIDMap[name] = id;
                return id;
            }
        }
    }
}
