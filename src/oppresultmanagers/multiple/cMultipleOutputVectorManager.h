//Copyright (c) 2016, CoRE Research Group, Hamburg University of Applied Sciences
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:
//
//1. Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//2. Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
//3. Neither the name of the copyright holder nor the names of its contributors
//   may be used to endorse or promote products derived from this software without
//   specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef CMULTIPLEOUTPUTVECTORMANAGER_H
#define CMULTIPLEOUTPUTVECTORMANAGER_H

#include <omnetpp.h>

//Std
#include <vector>

extern omnetpp::cConfigOption *CFGID_OUTPUTVECTORMANAGER_CLASSES;

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
        virtual bool record(void *vectorhandle, omnetpp::simtime_t t, double value) override;

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

