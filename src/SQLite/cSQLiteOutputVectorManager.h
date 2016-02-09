#ifndef __CSQLITEOUTPUTMGR_H
#define __CSQLITEOUTPUTMGR_H

#include <cSQLiteOutputManager.h>

class cSQLiteOutputVectorManager : public cOutputVectorManager, cSQLiteOutputManager
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

        sqlite3_stmt *insertVectorStmt;
        sqlite3_stmt *insertVectorAttrStmt;
        sqlite3_stmt *insertVectorDataStmt;


    public:
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
        const char *getFileName() const override;

        /**
         * Performs a database commit.
         */
        virtual void flush() override;
};

#endif

