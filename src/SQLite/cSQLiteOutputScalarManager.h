#ifndef CSQLITEOUTPUTSCALARMANAGER_H
#define CSQLITEOUTPUTSCALARMANAGER_H

#include <cSQLiteOutputManager.h>

class cSQLiteOutputScalarManager : public omnetpp::cIOutputScalarManager, cSQLiteOutputManager
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
        virtual void recordScalar(omnetpp::cComponent *component, const char *name, double value,
                omnetpp::opp_string_map *attributes = NULL) override;

        /**
         * Records a histogram or statistic object into the scalar result file.
         */
        virtual void recordStatistic(omnetpp::cComponent *component, const char *name, omnetpp::cStatistic *statistic,
                omnetpp::opp_string_map *attributes = NULL) override;

        virtual void flush() override;

        /**
         * Returns NULL, because this class doesn't use a file.
         */
        const char *getFileName() const override;

};

#endif

