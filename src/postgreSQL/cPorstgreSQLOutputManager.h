#ifndef CPORSTGRESQLOUTPUTMANAGER_H_
#define CPORSTGRESQLOUTPUTMANAGER_H_

#include <pqxx/pqxx>
#include <unordered_map>

#include <omnetpp.h>

class cPorstgreSQLOutputManager
{
    protected:
        pqxx::connection* connection;
        pqxx::nontransaction *transaction;

        size_t commitFreq;
        size_t insertCount;
        size_t runid;

        std::unordered_map<std::string, size_t> moduleIDMap;
        std::unordered_map<std::string, size_t> nameIDMap;

    public:
        /**
         * Constructor.
         */
        cPorstgreSQLOutputManager();

        /**
         * Destructor.
         */
        virtual ~cPorstgreSQLOutputManager();

        /**
         * Opens collecting. Called at the beginning of a simulation run.
         */
        virtual void startRun();

        /**
         * Closes collecting. Called at the end of a simulation run.
         */
        virtual void endRun();

        /**
         * Performs a database commit.
         */
        virtual void flush();

        /**
         * Returns NULL, because this class doesn't use a file.
         */
        const char *getFileName() const
        {
            return NULL;
        }
    protected:
        size_t getModuleID(std::string module);
        size_t getNameID(std::string name);
};

#endif /* CPORSTGRESQLOUTPUTMANAGER_H_ */
