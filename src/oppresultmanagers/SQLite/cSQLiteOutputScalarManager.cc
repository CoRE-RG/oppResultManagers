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

#include "oppresultmanagers/SQLite/cSQLiteOutputScalarManager.h"

#include <cmath>
#include <limits>

Register_Class(cSQLiteOutputScalarManager);

#define SQL_INSERT_SCALAR_RESULT  "INSERT INTO scalar(runid,moduleid,nameid,value) VALUES(?,?,?,?);"
#define SQL_INSERT_SCALAR_ATTR "INSERT INTO scalarattr(scalarid,nameid,value) VALUES(?,?,?);"
#define SQL_INSERT_STATISTIC "INSERT INTO statistic(runid,moduleid,nameid) VALUES(?,?,?);"
#define SQL_INSERT_STATISTIC_ATTR "INSERT INTO statisticattr(statisticid,nameid,value) VALUES(?,?,?);"
#define SQL_INSERT_FIELD "INSERT INTO field(statisticid,nameid,value) VALUES(?,?,?);"
#define SQL_INSERT_BIN "INSERT INTO bin(statisticid,binlowerbound,value) VALUES(?,?,?);"

namespace omnetpp {
namespace envir {
extern omnetpp::cConfigOption * CFGID_SCALAR_RECORDING;
}
}

void cSQLiteOutputScalarManager::startRun()
{
    cSQLiteOutputManager::startRun();
    char * zErrMsg = nullptr;
    int rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalar (\
                     id INTEGER PRIMARY KEY,\
                     runid INT NOT NULL,\
                     moduleid INT NOT NULL,\
                     nameid INT NOT NULL,\
                     value DOUBLE PRECISION,\
                     FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
                     FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
                     FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                     CONSTRAINT scalar_unique UNIQUE (runid,moduleid,nameid)\
                   );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(connection);
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'scalar': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS scalar_names AS \
                             SELECT scalar.id AS id, scalar.runid AS runid, module.name AS module,\
                             name.name AS name, scalar.value AS value FROM scalar \
                             JOIN module ON module.id = scalar.moduleid \
                             JOIN name ON name.id = scalar.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create view 'scalar_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS scalarattr(\
                                     id INTEGER PRIMARY KEY,\
                                     scalarid INT NOT NULL,\
                                     nameid INT NOT NULL,\
                                     value TEXT NOT NULL,\
                                     FOREIGN KEY (scalarid) REFERENCES scalar(id) ON DELETE CASCADE,\
                                     FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                                     CONSTRAINT scalarattr_unique UNIQUE (scalarid,nameid)\
                                  );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'scalarattr': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS scalarattr_names AS \
                                     SELECT scalarattr.id AS id, scalarattr.scalarid AS scalarid, \
                                     name.name AS name, scalarattr.value AS value FROM scalarattr \
                                     JOIN name ON name.id = scalarattr.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create view 'scalarattr_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS statistic(\
                                         id INTEGER PRIMARY KEY,\
                                         runid INT NOT NULL,\
                                         moduleid INT NOT NULL,\
                                         nameid INT NOT NULL,\
                                         FOREIGN KEY (runid) REFERENCES run(id) ON DELETE CASCADE,\
                                         FOREIGN KEY (moduleid) REFERENCES module(id) ON DELETE CASCADE,\
                                         FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                                         CONSTRAINT statistic_unique UNIQUE (runid,moduleid,nameid)\
                                      );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'statistic': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS statistic_names AS \
                                         SELECT statistic.id AS id, statistic.runid AS runid, \
                                         module.name AS module, name.name AS name FROM statistic \
                                         JOIN module ON module.id = statistic.moduleid \
                                         JOIN name ON name.id = statistic.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create view 'statistic_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS statisticattr(\
                                         id INTEGER PRIMARY KEY,\
                                         statisticid INT NOT NULL,\
                                         nameid INT NOT NULL,\
                                         value TEXT NOT NULL,\
                                         FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE,\
                                         FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                                         CONSTRAINT statisticattr_unique UNIQUE (statisticid,nameid)\
                                      );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'scalarattr': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS statisticattr_names AS \
                                         SELECT statisticattr.id AS id, statisticattr.statisticid AS statisticid, \
                                         name.name AS name, statisticattr.value AS value FROM statisticattr \
                                         JOIN name ON name.id = statisticattr.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create view 'scalarattr_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS field(\
                                             id INTEGER PRIMARY KEY,\
                                             statisticid INT NOT NULL,\
                                             nameid INT NOT NULL,\
                                             value DOUBLE PRECISION,\
                                             FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE,\
                                             FOREIGN KEY (nameid) REFERENCES name(id) ON DELETE CASCADE,\
                                             CONSTRAINT field_unique UNIQUE (statisticid,nameid)\
                                          );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'field': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE VIEW IF NOT EXISTS field_names AS \
                                 SELECT field.id AS id, field.statisticid AS statisticid,\
                                 name.name AS name, field.value AS value FROM field \
                                 JOIN name ON name.id = field.nameid;",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create view 'scalar_names': %s", zErrMsg);
    }
    rc =
            sqlite3_exec(connection,
                    "CREATE TABLE IF NOT EXISTS bin(\
                                                 id INTEGER PRIMARY KEY,\
                                                 statisticid INT NOT NULL,\
                                                 binlowerbound DOUBLE PRECISION,\
                                                 value INT NOT NULL,\
                                                 FOREIGN KEY (statisticid) REFERENCES statistic(id) ON DELETE CASCADE\
                                              );",
                    nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Can't create table 'field': %s", zErrMsg);
    }

    //prepare statements:
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_SCALAR_ATTR, strlen(SQL_INSERT_SCALAR_ATTR), &insertScalarAttrStmt,
            0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError(
                "cSQLiteOutputScalarManager:: Could not prepare statement (SQL_INSERT_SCALAR_ATTR): %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_STATISTIC, strlen(SQL_INSERT_STATISTIC), &insertStatisticStmt, 0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError(
                "cSQLiteOutputScalarManager:: Could not prepare statement (SQL_INSERT_STATISTIC): %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_STATISTIC_ATTR, strlen(SQL_INSERT_STATISTIC_ATTR),
            &insertStatisticAttrStmt, 0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError(
                "cSQLiteOutputScalarManager:: Could not prepare statement (SQL_INSERT_STATISTIC_ATTR): %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_FIELD, strlen(SQL_INSERT_FIELD), &insertFieldStmt, 0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not prepare statement (SQL_INSERT_FIELD): %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_prepare_v2(connection, SQL_INSERT_BIN, strlen(SQL_INSERT_BIN), &insertBinStmt, 0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not prepare statement (SQL_INSERT_BIN): %s",
                sqlite3_errmsg(connection));
    }
}

void cSQLiteOutputScalarManager::endRun()
{
    sqlite3_finalize(insertScalarAttrStmt);
    sqlite3_finalize(insertStatisticStmt);
    sqlite3_finalize(insertStatisticAttrStmt);
    sqlite3_finalize(insertFieldStmt);
    sqlite3_finalize(insertBinStmt);
    cSQLiteOutputManager::endRun();
}

void cSQLiteOutputScalarManager::recordScalar(omnetpp::cComponent *component, const char *name, double value,
        omnetpp::opp_string_map *attributes)
{

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare(connection, SQL_INSERT_SCALAR_RESULT, strlen(SQL_INSERT_SCALAR_RESULT), &stmt, 0);
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not prepare statement: %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(runid));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind active runnumber: %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(getModuleID(component->getFullPath())));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind module id: %s",
                sqlite3_errmsg(connection));
    }
    rc = sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(getNameID(name)));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind scalar name: %s",
                sqlite3_errmsg(connection));
    }
    if (std::isnan(value))
    {
        sqlite3_bind_null(stmt, 4);
    }
    else
    {
        rc = sqlite3_bind_double(stmt, 4, value);
    }
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value: %s",
                sqlite3_errmsg(connection));
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        throw omnetpp::cRuntimeError(
                "cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_SCALAR_RESULT): %s",
                sqlite3_errmsg(connection));
    }
    size_t scalarId = static_cast<size_t>(sqlite3_last_insert_rowid(connection));
    sqlite3_reset(stmt);

    if (attributes)
    {
        for (omnetpp::opp_string_map::iterator it = attributes->begin(); it != attributes->end(); ++it)
        {
            rc = sqlite3_bind_int64(insertScalarAttrStmt, 1, static_cast<sqlite3_int64>(scalarId));
            if (rc != SQLITE_OK)
            {
                throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind vectorid.");
            }
            rc = sqlite3_bind_int64(insertScalarAttrStmt, 2, static_cast<sqlite3_int64>(getNameID(it->first.c_str())));
            if (rc != SQLITE_OK)
            {
                throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind nameid.");
            }
            rc = sqlite3_bind_text(insertScalarAttrStmt, 3, it->second.c_str(), -1, SQLITE_STATIC);
            if (rc != SQLITE_OK)
            {
                throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value.");
            }

            rc = sqlite3_step(insertScalarAttrStmt);
            if (rc != SQLITE_DONE)
            {
                throw omnetpp::cRuntimeError(
                        "cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_SCALAR_ATTR): %s",
                        sqlite3_errmsg(connection));
            }
            sqlite3_clear_bindings(insertScalarAttrStmt);
            sqlite3_reset(insertScalarAttrStmt);
        }
    }
}

void cSQLiteOutputScalarManager::recordStatistic(omnetpp::cComponent *component, const char *name,
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
        int rc = sqlite3_bind_int64(insertStatisticStmt, 1, static_cast<sqlite3_int64>(runid));
        if (rc != SQLITE_OK)
        {
            throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind runid.");
        }
        rc = sqlite3_bind_int64(insertStatisticStmt, 2,
                static_cast<sqlite3_int64>(getModuleID(component->getFullPath().c_str())));
        if (rc != SQLITE_OK)
        {
            throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind moduleid.");
        }
        rc = sqlite3_bind_int64(insertStatisticStmt, 3, static_cast<sqlite3_int64>(getNameID(name)));
        if (rc != SQLITE_OK)
        {
            throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind nameid.");
        }

        rc = sqlite3_step(insertStatisticStmt);
        if (rc != SQLITE_DONE)
        {
            throw omnetpp::cRuntimeError(
                    "cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_STATISTIC): %s",
                    sqlite3_errmsg(connection));
        }
        size_t statisticId = static_cast<size_t>(sqlite3_last_insert_rowid(connection));
        sqlite3_clear_bindings(insertStatisticStmt);
        sqlite3_reset(insertStatisticStmt);

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
                rc = sqlite3_bind_int64(insertStatisticAttrStmt, 1, static_cast<sqlite3_int64>(statisticId));
                if (rc != SQLITE_OK)
                {
                    throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind statisticId.");
                }
                rc = sqlite3_bind_int64(insertStatisticAttrStmt, 2,
                        static_cast<sqlite3_int64>(getNameID(it->first.c_str())));
                if (rc != SQLITE_OK)
                {
                    throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind nameid.");
                }
                rc = sqlite3_bind_text(insertStatisticAttrStmt, 3, it->second.c_str(), -1, SQLITE_STATIC);
                if (rc != SQLITE_OK)
                {
                    throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value.");
                }

                rc = sqlite3_step(insertStatisticAttrStmt);
                if (rc != SQLITE_DONE)
                {
                    throw omnetpp::cRuntimeError(
                            "cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_STATISTIC_ATTR): %s",
                            sqlite3_errmsg(connection));
                }
                sqlite3_clear_bindings(insertStatisticAttrStmt);
                sqlite3_reset(insertStatisticAttrStmt);
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

void cSQLiteOutputScalarManager::insertField(size_t statisticId, size_t nameid, double value)
{
    int rc = sqlite3_bind_int64(insertFieldStmt, 1, static_cast<sqlite3_int64>(statisticId));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind statisticid.");
    }
    rc = sqlite3_bind_int64(insertFieldStmt, 2, static_cast<sqlite3_int64>(nameid));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind nameid.");
    }
    if (std::isnan(value))
    {
        sqlite3_bind_null(insertFieldStmt, 3);
    }
    else
    {
        rc = sqlite3_bind_double(insertFieldStmt, 3, value);
    }
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value.");
    }
    rc = sqlite3_step(insertFieldStmt);
    if (rc != SQLITE_DONE)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_FIELD): %s",
                sqlite3_errmsg(connection));
    }
    sqlite3_clear_bindings(insertFieldStmt);
    sqlite3_reset(insertFieldStmt);
}

void cSQLiteOutputScalarManager::insertBin(size_t statisticId, double binlowerbound, size_t value)
{
    int rc = sqlite3_bind_int64(insertBinStmt, 1, static_cast<sqlite3_int64>(statisticId));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind statisticId.");
    }
    if (std::isnan(binlowerbound))
    {
        sqlite3_bind_null(insertBinStmt, 2);
    }
    else
    {
        rc = sqlite3_bind_double(insertBinStmt, 2, binlowerbound);
    }
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind binlowerbound.");
    }
    rc = sqlite3_bind_int64(insertBinStmt, 3, static_cast<sqlite3_int64>(value));
    if (rc != SQLITE_OK)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not bind value.");
    }

    rc = sqlite3_step(insertBinStmt);
    if (rc != SQLITE_DONE)
    {
        throw omnetpp::cRuntimeError("cSQLiteOutputScalarManager:: Could not execute statement (SQL_INSERT_BIN): %s",
                sqlite3_errmsg(connection));
    }
    sqlite3_clear_bindings(insertBinStmt);
    sqlite3_reset(insertBinStmt);
}

void cSQLiteOutputScalarManager::flush()
{
    cSQLiteOutputManager::flush();
}

const char *cSQLiteOutputScalarManager::getFileName() const
{
    return cSQLiteOutputManager::getFileName();
}
