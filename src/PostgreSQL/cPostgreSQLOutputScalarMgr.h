#ifndef __MYSQLOUTPUTSCALARMGR_H
#define __MYSQLOUTPUTSCALARMGR_H

#include <pqxx/pqxx>
#include <unordered_map>

#include <omnetpp.h>

class cPostgreSQLOutputScalarMgr : public cOutputScalarManager
{
  private:
    pqxx::connection* connection;
    pqxx::nontransaction *transaction;

    size_t commitFreq;
    size_t insertCount;
    size_t runid;

    std::unordered_map<std::string,size_t> moduleIDMap;
    std::unordered_map<std::string,size_t> nameIDMap;

  public:
    /**
     * Constructor.
     */
    explicit cPostgreSQLOutputScalarMgr();

    /**
     * Destructor.
     */
    virtual ~cPostgreSQLOutputScalarMgr();

    /**
     * Opens collecting. Called at the beginning of a simulation run.
     */
    virtual void startRun() override;

    /**
     * Closes collecting. Called at the end of a simulation run.
     */
    virtual void endRun() override;


    /**
     * Records a double scalar result into the scalar result file.
     */
    virtual void recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes=NULL) override;

    /**
     * Records a histogram or statistic object into the scalar result file.
     */
    virtual void recordStatistic(cComponent *component, const char *name, cStatistic *statistic, opp_string_map *attributes=NULL) override;

    /**
     * Returns NULL, because this class doesn't use a file.
     */
    const char *getFileName() const override {return NULL;}

    /**
     * Performs a database commit.
     */
    virtual void flush() override;

  private:
    size_t getModuleID(std::string module);
    size_t getNameID(std::string name);
};

#endif

