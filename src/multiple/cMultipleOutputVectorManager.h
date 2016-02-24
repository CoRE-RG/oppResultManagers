#ifndef CMULTIPLEOUTPUTVECTORMANAGER_H
#define CMULTIPLEOUTPUTVECTORMANAGER_H

#include <omnetpp.h>

//Std
#include <vector>

class cMultipleOutputVectorManager : public omnetpp::cIOutputVectorManager
{
    protected:
        struct sVectorData
        {
                std::vector<void *> vectorhandles;
        };
    protected:
        std::vector<omnetpp::cIOutputVectorManager *> vectorOutputManagers;
    public:
        /**
         * Constructor.
         */
        cMultipleOutputVectorManager();

        /**
         * Destructor.
         */
        virtual ~cMultipleOutputVectorManager();

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
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const override;

        /**
         * Performs a database commit.
         */
        virtual void flush() override;
};

#endif

