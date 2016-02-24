#ifndef CSQLITEOUTPUTVECTORMANAGER_H
#define CSQLITEOUTPUTVECTORMANAGER_H

#include "cDatabaseOutputVectorManager.h"
#include <cSQLiteOutputManager.h>

class cSQLiteOutputVectorManager : public cDatabaseOutputVectorManager, cSQLiteOutputManager
{

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
         * Writes the (time, value) pair into the output file.
         */
        virtual bool record(void *vectorhandle, simtime_t t, double value) override;

        /**
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const override;

        /**
         * Performs a database commit.
         */
        virtual void flush() override;
};

#endif

