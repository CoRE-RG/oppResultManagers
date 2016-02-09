#ifndef __MYSQLOUTPUTSCALARMGR_H
#define __MYSQLOUTPUTSCALARMGR_H

#include <cSQLiteOutputManager.h>

class cSQLiteOutputScalarManager : public cOutputScalarManager, cSQLiteOutputManager
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
         * Records a double scalar result into the scalar result file.
         */
        virtual void recordScalar(cComponent *component, const char *name, double value, opp_string_map *attributes =
                NULL) override;

        /**
         * Records a histogram or statistic object into the scalar result file.
         */
        virtual void recordStatistic(cComponent *component, const char *name, cStatistic *statistic,
                opp_string_map *attributes = NULL) override;

        virtual void flush() override;

        /**
         * Returns NULL, because this class doesn't use a file.
         */
        const char *getFileName() const override;

};

#endif
