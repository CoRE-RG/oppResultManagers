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

#include "oppresultmanagers/postgreSQL/cPostgreSQLOutputVectorManager.h"

#include "oppresultmanagers/utilities/HelperFunctions.h"

Register_Class(cPostgreSQLOutputVectorManager);

#define SQL_INSERT_VECTOR "INSERT INTO vector(runid, moduleid, nameid) VALUES($1,$2,$3) RETURNING id"
#define SQL_INSERT_VECTOR_ATTR "INSERT INTO vectorattr(vectorid,nameid,value) VALUES($1,$2,$3)"
#define SQL_INSERT_VECTOR_DATA  "INSERT INTO vectordata(vectorid,time,value) VALUES($1,$2,$3)"

void cPostgreSQLOutputVectorManager::startRun()
{
    cPorstgreSQLOutputManager::startRun();

    //Create Tables
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vector(\
         id SERIAL NOT NULL PRIMARY KEY,\
         runid INT NOT NULL,\
         moduleid INT NOT NULL,\
         nameid INT NOT NULL,\
         FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
         FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
         FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
         CONSTRAINT vector_unique UNIQUE(runid, moduleid, nameid) \
      );");
    transaction->exec(
                "CREATE OR REPLACE VIEW vector_names AS \
         SELECT vector.id AS id,runid,module.name AS module,name.name AS name FROM vector \
         JOIN module ON module.id = vector.moduleid \
         JOIN name ON name.id = vector.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vectorattr(\
         id SERIAL NOT NULL PRIMARY KEY,\
         vectorid INT NOT NULL,\
         nameid INT NOT NULL,\
         value TEXT NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id) ON DELETE CASCADE,\
         FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE\
      );");
    transaction->exec(
                "CREATE OR REPLACE VIEW vectorattr_names AS \
         SELECT vectorattr.id AS id, vectorattr.vectorid AS vectorid, \
         name.name AS name, vectorattr.value AS value FROM vectorattr \
         JOIN name ON name.id = vectorattr.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS vectordata (\
         vectorid INT NOT NULL,\
         time DOUBLE PRECISION NOT NULL,\
         value DOUBLE PRECISION NOT NULL,\
         FOREIGN KEY (vectorid) REFERENCES vector(id) ON DELETE CASCADE\
       );");
    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputVectorManager::endRun()
{
    cPorstgreSQLOutputManager::endRun();
}

bool cPostgreSQLOutputVectorManager::record(void *vectorhandle, omnetpp::simtime_t t, double value)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);

    if (vp->enabled && inIntervals(t, vp->intervals))
    {
        if (!vp->initialised)
        {
            pqxx::result result = transaction->parameterized(SQL_INSERT_VECTOR)(runid)(
                    getModuleID(vp->modulename.c_str()))(getNameID(vp->vectorname.c_str())).exec();
            if (result.size() != 1)
            {
                throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarMgr:: internal error!");
            }
            vp->id = result[0][0].as<long>();
            vp->initialised = true;

            for (omnetpp::opp_string_map::iterator it = vp->attributes.begin(); it != vp->attributes.end(); ++it)
            {
                transaction->parameterized(SQL_INSERT_VECTOR_ATTR)(vp->id)(getNameID(it->first.c_str()))(
                        it->second.c_str()).exec();
            }
        }

        // fill in prepared statement parameters, and fire off the statement
        transaction->parameterized(SQL_INSERT_VECTOR_DATA)(vp->id)(SIMTIME_DBL(t))(value).exec();

        // commit every once in a while
        if (commitFreq && ((++insertCount % commitFreq) == 0))
        {
            if (transaction)
            {
                transaction->exec("COMMIT; BEGIN;");
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void cPostgreSQLOutputVectorManager::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputVectorManager::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
