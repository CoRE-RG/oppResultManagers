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

#include "oppresultmanagers/postgreSQL/cPostgreSQLOutputScalarManager.h"

Register_Class(cPostgreSQLOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES($1,$2,$3,$4) RETURNING id"
#define SQL_INSERT_SCALAR_ATTR "INSERT INTO scalarattr(scalarid,nameid,value) VALUES($1,$2,$3);"
#define SQL_INSERT_STATISTIC "INSERT INTO statistic(runid,moduleid,nameid) VALUES($1,$2,$3) RETURNING id;"
#define SQL_INSERT_STATISTIC_ATTR "INSERT INTO statisticattr(statisticid,nameid,value) VALUES($1,$2,$3);"
#define SQL_INSERT_FIELD "INSERT INTO field(statisticid,nameid,value) VALUES($1,$2,$3);"
#define SQL_INSERT_BIN "INSERT INTO bin(statisticid,binlowerbound,value) VALUES($1,$2,$3);"

namespace omnetpp {
namespace envir {
extern omnetpp::cConfigOption * CFGID_SCALAR_RECORDING;
}
}

void cPostgreSQLOutputScalarManager::startRun()
{
    cPorstgreSQLOutputManager::startRun();
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS scalar (\
             id SERIAL PRIMARY KEY,\
             runid INT NOT NULL,\
             moduleid INT NOT NULL,\
             nameid INT NOT NULL,\
             value DOUBLE PRECISION NOT NULL,\
             FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
             FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT scalar_unique UNIQUE (runid,moduleid,nameid)\
           );");
    transaction->exec(
            "CREATE OR REPLACE VIEW scalar_names AS \
             SELECT scalar.id AS id, scalar.runid AS runid, module.name AS module,\
             name.name AS name, scalar.value AS value FROM scalar \
             JOIN module ON module.id = scalar.moduleid \
             JOIN name ON name.id = scalar.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS scalarattr(\
             id SERIAL PRIMARY KEY,\
             scalarid INT NOT NULL,\
             nameid INT NOT NULL,\
             value TEXT NOT NULL,\
             FOREIGN KEY (scalarid) REFERENCES scalar(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT scalarattr_unique UNIQUE (scalarid,nameid)\
          );");
    transaction->exec(
            "CREATE OR REPLACE VIEW scalarattr_names AS \
             SELECT scalarattr.id AS id, scalarattr.scalarid AS scalarid, \
             name.name AS name, scalarattr.value AS value FROM scalarattr \
             JOIN name ON name.id = scalarattr.nameid;");

    transaction->exec(
            "CREATE TABLE IF NOT EXISTS statistic(\
             id SERIAL PRIMARY KEY,\
             runid INT NOT NULL,\
             moduleid INT NOT NULL,\
             nameid INT NOT NULL,\
             FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
             FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT statistic_unique UNIQUE (runid,moduleid,nameid)\
          );");
    transaction->exec(
            "CREATE OR REPLACE VIEW statistic_names AS \
             SELECT statistic.id AS id, statistic.runid AS runid, \
             module.name AS module, name.name AS name FROM statistic \
             JOIN module ON module.id = statistic.moduleid \
             JOIN name ON name.id = statistic.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS statisticattr(\
             id SERIAL PRIMARY KEY,\
             statisticid INT NOT NULL,\
             nameid INT NOT NULL,\
             value TEXT NOT NULL,\
             FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT statisticattr_unique UNIQUE (statisticid,nameid)\
          );");
    transaction->exec(
            "CREATE OR REPLACE VIEW statisticattr_names AS \
             SELECT statisticattr.id AS id, statisticattr.statisticid AS statisticid, \
             name.name AS name, statisticattr.value AS value FROM statisticattr \
             JOIN name ON name.id = statisticattr.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS field(\
             id SERIAL PRIMARY KEY,\
             statisticid INT NOT NULL,\
             nameid INT NOT NULL,\
             value DOUBLE PRECISION,\
             FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE,\
             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
             CONSTRAINT field_unique UNIQUE (statisticid,nameid)\
          );");
    transaction->exec(
            "CREATE OR REPLACE VIEW field_names AS \
             SELECT field.id AS id, field.statisticid AS statisticid,\
             name.name AS name, field.value AS value FROM field \
             JOIN name ON name.id = field.nameid;");
    transaction->exec(
            "CREATE TABLE IF NOT EXISTS bin(\
             id SERIAL PRIMARY KEY,\
             statisticid INT NOT NULL,\
             binlowerbound DOUBLE PRECISION,\
             value INT NOT NULL,\
             FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE\
          );");

    transaction->exec("COMMIT; BEGIN;");
}

void cPostgreSQLOutputScalarManager::endRun()
{
    cPorstgreSQLOutputManager::endRun();
}

void cPostgreSQLOutputScalarManager::recordScalar(omnetpp::cComponent *component, const char *name, double value,
        omnetpp::opp_string_map *attributes)
{

    pqxx::result result = transaction->parameterized(SQL_INSERT_SCALAR_RESULT)(runid)(
            getModuleID(component->getFullPath()))(getNameID(name))(value).exec();
    if (result.size() != 1)
    {
        throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarManager:: internal error!");
    }
    size_t scalarId = result[0][0].as<size_t>();

    if (attributes)
    {
        for (omnetpp::opp_string_map::const_iterator it = attributes->begin(); it != attributes->end(); ++it)
        {
            transaction->parameterized(SQL_INSERT_SCALAR_ATTR)(scalarId)(getNameID(it->first.c_str()))(
                    it->second.c_str()).exec();
        }
    }

    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        if (transaction)
        {
            transaction->exec("COMMIT; BEGIN;");
        }
    }
}

void cPostgreSQLOutputScalarManager::recordStatistic(omnetpp::cComponent *component, const char *name,
        omnetpp::cStatistic *statistic, omnetpp::opp_string_map *attributes)
{
    if (!name)
        name = statistic->getFullName();
    if (!name || !name[0])
        name = "(unnamed)";

    // check that recording this statistic is not disabled as a whole
    std::string objectFullPath = component->getFullPath() + "." + name;

    bool enabled = omnetpp::getEnvir()->getConfig()->getAsBool(objectFullPath.c_str(),
            omnetpp::envir::CFGID_SCALAR_RECORDING);
    if (enabled)
    {
        pqxx::result result = transaction->parameterized(SQL_INSERT_STATISTIC)(runid)(
                getModuleID(component->getFullPath()))(getNameID(name)).exec();
        if (result.size() != 1)
        {
            throw omnetpp::cRuntimeError("cPostgreSQLOutputScalarManager:: internal error!");
        }
        size_t statisticId = result[0][0].as<size_t>();

        insertField(statisticId, getNameID("max"), statistic->getMax());
        insertField(statisticId, getNameID("min"), statistic->getMin());
        insertField(statisticId, getNameID("mean"), statistic->getMean());
        insertField(statisticId, getNameID("count"), static_cast<double>(statistic->getCount()));
        insertField(statisticId, getNameID("stddev"), statistic->getStddev());
        insertField(statisticId, getNameID("sum"), statistic->getSum());
        insertField(statisticId, getNameID("sqrsum"), statistic->getSqrSum());
        if (statistic->isWeighted())
        {
            insertField(statisticId, getNameID("weights"), statistic->getWeights());
            insertField(statisticId, getNameID("weightedSum"), statistic->getWeightedSum());
            insertField(statisticId, getNameID("sqrSumWeights"), statistic->getSqrSumWeights());
            insertField(statisticId, getNameID("weightedSqrSum"), statistic->getWeightedSqrSum());
        }

        if (attributes)
        {
            for (omnetpp::opp_string_map::iterator it = attributes->begin(); it != attributes->end(); ++it)
            {
                transaction->parameterized(SQL_INSERT_STATISTIC_ATTR)(statisticId)(getNameID(it->first.c_str()))(
                        it->second.c_str()).exec();
            }
        }
        if (omnetpp::cDensityEstBase *histogram = dynamic_cast<omnetpp::cDensityEstBase *>(statistic))
        {
            // check that recording the histogram is enabled
            bool hist_enabled = omnetpp::getEnvir()->getConfig()->getAsBool((objectFullPath + ":histogram").c_str(),
                    omnetpp::envir::CFGID_SCALAR_RECORDING);
            if (hist_enabled)
            {
                if (!histogram->isTransformed())
                    histogram->transform();

                int n = histogram->getNumCells();
                if (n > 0)
                {
                    //Insert underflow cell
                    insertBin(statisticId, std::numeric_limits<double>::quiet_NaN(), histogram->getUnderflowCell());
                    //Insert bins
                    for (int i = 0; i < n; i++)
                    {
                        insertBin(statisticId, histogram->getBasepoint(i),
                                static_cast<size_t>(histogram->getCellValue(i)));
                    }
                    //Insert overflow cell
                    insertBin(statisticId, histogram->getBasepoint(n), histogram->getOverflowCell());
                }
            }
        }
    }
    // commit every once in a while
    if (commitFreq && ((++insertCount % commitFreq) == 0))
    {
        flush();
    }
}

void cPostgreSQLOutputScalarManager::insertField(size_t statisticId, size_t nameid, double value)
{
    transaction->parameterized(SQL_INSERT_FIELD)(statisticId)(nameid)(value).exec();
}

void cPostgreSQLOutputScalarManager::insertBin(size_t statisticId, double binlowerbound, size_t value)
{
    transaction->parameterized(SQL_INSERT_BIN)(statisticId)(binlowerbound)(value).exec();
}

void cPostgreSQLOutputScalarManager::flush()
{
    cPorstgreSQLOutputManager::flush();
}

const char *cPostgreSQLOutputScalarManager::getFileName() const
{
    return cPorstgreSQLOutputManager::getFileName();
}
