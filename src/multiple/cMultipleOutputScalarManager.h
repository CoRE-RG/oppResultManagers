#ifndef CMULTIPLEOUTPUTSCALARMANAGER_H
#define CMULTIPLEOUTPUTSCALARMANAGER_H

#include <omnetpp.h>

//Std
#include <vector>

class cMultipleOutputScalarManager : public omnetpp::cIOutputScalarManager
{
    protected:
        std::vector<omnetpp::cIOutputScalarManager *> scalarOutputManagers;
    public:
        /**
         * Constructor.
         */
        cMultipleOutputScalarManager();

        /**
         * Destructor.
         */
        virtual ~cMultipleOutputScalarManager();

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

