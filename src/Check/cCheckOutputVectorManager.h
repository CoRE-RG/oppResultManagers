#ifndef CCHECKOUTPUTVECTORMANAGER_H
#define CCHECKOUTPUTVECTORMANAGER_H

#include <omnetpp.h>

//Std
#include <vector>

class cCheckOutputVectorManager : public cOutputVectorManager
{
    protected:
        struct sVectorData
        {
        };
    public:
        /**
         * Constructor.
         */
        cCheckOutputVectorManager();

        /**
         * Destructor.
         */
        virtual ~cCheckOutputVectorManager();

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

