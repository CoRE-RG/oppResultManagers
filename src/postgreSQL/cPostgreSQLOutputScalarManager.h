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

