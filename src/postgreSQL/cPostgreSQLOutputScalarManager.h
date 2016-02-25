#ifndef CPOSTGRESQLOUTPUTSCALARMANAGER_H
#define CPOSTGRESQLOUTPUTSCALARMANAGER_H

#include <cPorstgreSQLOutputManager.h>

class cPostgreSQLOutputScalarManager : public omnetpp::cIOutputScalarManager, cPorstgreSQLOutputManager
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
                omnetpp::opp_string_map *attributes = nullptr) override;

        /**
         * Records a histogram or statistic object into the scalar result file.
         */
        virtual void recordStatistic(omnetpp::cComponent *component, const char *name, omnetpp::cStatistic *statistic,
                omnetpp::opp_string_map *attributes = nullptr) override;

        virtual void flush() override;

        /**
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const override;

    private:
        void insertField(size_t statisticId, size_t nameid, double value);
        void insertBin(size_t statisticId, double binlowerbound, size_t value);
};

#endif

