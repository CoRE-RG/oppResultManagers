#include "cPostgreSQLOutputVectorMgr.h"
#include "opppostgresqlutils.h"

Register_Class(cPostgreSQLOutputVectorMgr);
Register_GlobalConfigOption(CFGID_POSTGRESQLOUTVECTORMGR_CONNECTION, "postgresqloutputvectormanager-connection",
        CFG_STRING, "\"\"", "Object name of database connection parameters");
Register_GlobalConfigOption(CFGID_POSTGRESQLOUTVECTORMGR_COMMIT_FREQ, "postgresqloutputvectormanager-commit-freq",
        CFG_INT, "10", "COMMIT every n INSERTs, default=10");

#define SQL_INSERT_RUN "INSERT INTO run(runnumber,network) VALUES($1,$2) RETURNING id"
#define SQL_INSERT_MODULE "INSERT INTO module(name) VALUES($1) RETURNING id"
#define SQL_INSERT_NAME "INSERT INTO name(name) VALUES($1) RETURNING id"
#define SQL_INSERT_VECTOR "INSERT INTO vector(runid, moduleid, nameid) VALUES($1,$2,$3) RETURNING id"
#define SQL_INSERT_VECTOR_ATTR "INSERT INTO vectorattr(vectorid,nameid,value) VALUES($1,$2,$3)"
#define SQL_INSERT_VECTOR_DATA  "INSERT INTO vectordata(vectorid,time,value) VALUES($1,$2,$3)"
#define SQL_SELECT_MODULE "SELECT * FROM module;"
#define SQL_SELECT_VECTOR_NAME "SELECT * FROM name;"

cPostgreSQLOutputVectorMgr::cPostgreSQLOutputVectorMgr()
{
    connection = nullptr;
    transaction = nullptr;
    runid = 0;
}

cPostgreSQLOutputVectorMgr::~cPostgreSQLOutputVectorMgr()
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

void cPostgreSQLOutputVectorMgr::startRun()
{
    if (!connection)
    {
        std::string cfgobj = ev.getConfig()->getAsString(CFGID_POSTGRESQLOUTVECTORMGR_CONNECTION);
        connection = new pqxx::connection(cfgobj.c_str());
    }

    commitFreq = ev.getConfig()->getAsInt(CFGID_POSTGRESQLOUTVECTORMGR_COMMIT_FREQ);

    pqxx::work work_transaction(*connection);
    //Create Tables
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS run (\
         id SERIAL NOT NULL PRIMARY KEY,\
         runnumber BIGINT NOT NULL,\
         network TEXT NOT NULL,\
         date TIMESTAMP NOT NULL DEFAULT now()\
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
            "CREATE TABLE IF NOT EXISTS vector(\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid INT NOT NULL,\
         moduleid INT NOT NULL,\
         nameid INT NOT NULL,\
         FOREIGN KEY (runid) REFERENCES run(id),\
         FOREIGN KEY (moduleid) REFERENCES module(id),\
         FOREIGN KEY (nameid) REFERENCES name(id)\
      );");
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS vectorattr(\
         id SERIAL NOT NULL PRIMARY KEY,\
         vectorid INT NOT NULL,\
         nameid INT NOT NULL,\
         value TEXT NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id),\
         FOREIGN KEY (nameid) REFERENCES name(id)\
      );");
    work_transaction.exec(
            "CREATE TABLE IF NOT EXISTS vectordata (\
         vectorid INT NOT NULL,\
         time DOUBLE PRECISION NOT NULL,\
         value DOUBLE PRECISION NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id)\
       );");

    pqxx::result result = work_transaction.parameterized(SQL_INSERT_RUN)(
    simulation.getActiveEnvir()->getConfigEx()->getActiveRunNumber())(
    simulation.getNetworkType()->getName()).exec();
    if (result.size() != 1)
    {
        throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
    }
    runid = result[0][0].as<size_t>();

    //Find already existing modules and scalars
    result = work_transaction.exec(SQL_SELECT_MODULE);
    for (size_t rownum = 0; rownum < result.size(); ++rownum)
    {
        ev << "moduleIDMap: " << result[rownum][1].as<const char*>() << "; id: " << result[rownum][0].as<size_t>()
                << endl;
        moduleIDMap[result[rownum][1].as<std::string>()] = result[rownum][0].as<size_t>();
    }
    result = work_transaction.exec(SQL_SELECT_VECTOR_NAME);
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

void cPostgreSQLOutputVectorMgr::endRun()
{
    if (transaction)
    {
        transaction->exec("COMMIT;");
    }
}

void *cPostgreSQLOutputVectorMgr::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;
    vp->enabled = true;
    return vp;
}
void cPostgreSQLOutputVectorMgr::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *) vectorhandle;
    delete vp;
}
void cPostgreSQLOutputVectorMgr::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    ASSERT(vectorhandle != NULL);
    sVectorData *vp = (sVectorData *) vectorhandle;
    vp->attributes[name] = value;
}
bool cPostgreSQLOutputVectorMgr::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *) vectorhandle;

    if (!vp->enabled)
        return false;

    if (!vp->initialised)
    {
        pqxx::result result = transaction->parameterized(SQL_INSERT_VECTOR)(runid)(getModuleID(vp->modulename.c_str()))(
                getNameID(vp->vectorname.c_str())).exec();
        if (result.size() != 1)
        {
            throw cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
        }
        vp->id = result[0][0].as<long>();
        vp->initialised = true;

        for (opp_string_map::iterator it = vp->attributes.begin(); it != vp->attributes.end(); ++it)
        {
            transaction->parameterized(SQL_INSERT_VECTOR_ATTR)(vp->id)(getNameID(it->first.c_str()))(it->second.c_str()).exec();
        }
    }

    // fill in prepared statement parameters, and fire off the statement
    transaction->parameterized(SQL_INSERT_VECTOR_DATA)(vp->id)(SIMTIME_DBL(t))(value).exec();

    // commit every once in a while
    if ((++insertCount % commitFreq) == 0)
    {
        if (transaction)
        {
            transaction->exec("COMMIT; BEGIN;");
        }
    }
    return true;
}

void cPostgreSQLOutputVectorMgr::flush()
{
    if (transaction)
    {
        transaction->exec("COMMIT; BEGIN;");
    }
}

size_t cPostgreSQLOutputVectorMgr::getModuleID(std::string module)
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
        ev << "Inserted module " << module << "in table modules. Id is: " << id << endl;
        moduleIDMap[module] = id;
        transaction->exec("COMMIT; BEGIN;");
        return id;
    }
}

size_t cPostgreSQLOutputVectorMgr::getNameID(std::string name)
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
