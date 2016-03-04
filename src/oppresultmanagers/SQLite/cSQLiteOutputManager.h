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

#ifndef CSQLITEOUTPUTMANAGER_H_
#define CSQLITEOUTPUTMANAGER_H_

#include <sqlite3.h>
#include <unordered_map>

#include <omnetpp.h>

#define SCHEMAVERSION 2

extern omnetpp::cConfigOption *CFGID_SQLITEOUTMGR_FILE;
extern omnetpp::cConfigOption *CFGID_SQLITEMGR_COMMIT_FREQ;

class cSQLiteOutputManager
{
    protected:
        static sqlite3* connection;
        static bool hasTransaction;
        static size_t users;

        size_t commitFreq;
        size_t insertCount;
        size_t runid;

        static std::unordered_map<std::string, size_t> moduleIDMap;
        static std::unordered_map<std::string, size_t> nameIDMap;

    public:
        /**
         * Constructor.
         */
        cSQLiteOutputManager();

        /**
         * Destructor.
         */
        virtual ~cSQLiteOutputManager();

        /**
         * Opens collecting. Called at the beginning of a simulation run.
         */
        virtual void startRun();

        /**
         * Closes collecting. Called at the end of a simulation run.
         */
        virtual void endRun();

        /**
         * Performs a database commit.
         */
        virtual void flush();

        /**
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const
        {
            return nullptr;
        }
    protected:
        size_t getModuleID(std::string module);
        size_t getNameID(std::string name);
};

#endif /* CSQLITEOUTPUTMANAGER_H_ */
