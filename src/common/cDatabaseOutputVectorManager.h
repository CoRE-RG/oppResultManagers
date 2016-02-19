#ifndef __CDATABASEOUTPUTVECTORMANAGER_H
#define __CDATABASEOUTPUTVECTORMANAGER_H

#include <omnetpp.h>

class cDatabaseOutputVectorManager : public omnetpp::cIOutputVectorManager
{
    protected:
        struct sVectorData
        {
                long id;             // vector ID
                omnetpp::opp_string modulename; // module of cOutVector object
                omnetpp::opp_string vectorname; // cOutVector object name
                omnetpp::opp_string_map attributes; // vector attributes
                bool initialised;    // true if the "label" line is already written out
                bool enabled;        // write to the output file can be enabled/disabled
                bool recordEventNumbers;  // write to the output file can be enabled/disabled
                std::vector<std::pair<simtime_t, simtime_t>> intervals; // write begins at starttime
        };

    public:

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

};

#endif

