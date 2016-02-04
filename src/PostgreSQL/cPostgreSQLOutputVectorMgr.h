#ifndef __MYSQLOUTPUTSCALARMGR_H
#define __MYSQLOUTPUTSCALARMGR_H

#include <pqxx/pqxx>
#include <unordered_map>

#include <omnetpp.h>

class cPostgreSQLOutputVectorMgr : public cOutputVectorManager
{
    protected:
        struct sVectorData
        {
                long id;             // vector ID
                opp_string modulename; // module of cOutVector object
                opp_string vectorname; // cOutVector object name
                opp_string_map attributes; // vector attributes
                bool initialised;    // true if the "label" line is already written out
                bool enabled;        // write to the output file can be enabled/disabled
                bool recordEventNumbers;  // write to the output file can be enabled/disabled
        };
    private:
        pqxx::connection* connection;
        pqxx::nontransaction *transaction;

        size_t commitFreq;
        size_t insertCount;
        size_t runid;

        std::unordered_map<std::string, size_t> moduleIDMap;
        std::unordered_map<std::string,size_t> nameIDMap;

    public:
        /**
         * Constructor.
         */
        explicit cPostgreSQLOutputVectorMgr();

        /**
         * Destructor.
         */
        virtual ~cPostgreSQLOutputVectorMgr();

        /**
         * Opens collecting. Called at the beginning of a simulation run.
         */
        virtual void startRun() override;

        /**
         * Closes collecting. Called at the end of a simulation run.
         */
        virtual void endRun() override;

        /**
         * Registers a vector and returns a handle.
         */
        virtual void *registerVector(const char *modulename, const char *vectorname) override;

        /**
         * Deregisters the output vector.
         */
        virtual void deregisterVector(void *vectorhandle) override;

        /**
         * Sets an attribute of an output vector.
         */
        virtual void setVectorAttribute(void *vectorhandle, const char *name, const char *value) override;

        /**
         * Writes the (time, value) pair into the output file.
         */
        virtual bool record(void *vectorhandle, simtime_t t, double value) override;

        /**
         * Returns NULL, because this class doesn't use a file.
         */
        const char *getFileName() const override
        {
            return NULL;
        }

        /**
         * Performs a database commit.
         */
        virtual void flush() override;

    private:
        size_t getModuleID(std::string module);
        size_t getNameID(std::string name);
};

#endif

