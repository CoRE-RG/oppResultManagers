#ifndef CPOSTGRESQLOUTPUTVECTORMANAGER_H
#define CPOSTGRESQLOUTPUTVECTORMANAGER_H

#include "cDatabaseOutputVectorManager.h"
#include <cPorstgreSQLOutputManager.h>

class cPostgreSQLOutputVectorMgr : public cDatabaseOutputVectorManager, cPorstgreSQLOutputManager
{
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

