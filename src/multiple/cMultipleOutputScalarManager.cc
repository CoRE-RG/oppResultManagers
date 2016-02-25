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

#include "cMultipleOutputScalarManager.h"

Register_Class(cMultipleOutputScalarManager);

Register_PerRunConfigOption(CFGID_OUTPUTSCALARMANAGER_CLASSES, "outputscalarmanager-classes", CFG_STRING, "\"\"",
        "List of OutputScalarManager Classes (comma or space separated)");

cMultipleOutputScalarManager::cMultipleOutputScalarManager()
{
    std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_OUTPUTSCALARMANAGER_CLASSES);

    std::vector<std::string> managerClasses = omnetpp::cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator managerClass = managerClasses.begin();
            managerClass != managerClasses.end(); ++managerClass)
    {
        omnetpp::cObject *outscalarmgr_tmp = omnetpp::createOne((*managerClass).c_str());
        if (omnetpp::cIOutputScalarManager * scalarManager =
                dynamic_cast<omnetpp::cIOutputScalarManager *>(outscalarmgr_tmp))
        {
            scalarOutputManagers.push_back(scalarManager);
        }
        else
        {
            throw omnetpp::cRuntimeError("Class \"%s\" is not subclassed from cOutputScalarManager",
                    (*managerClass).c_str());
        }
    }
}

cMultipleOutputScalarManager::~cMultipleOutputScalarManager()
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end();)
    {
        scalarOutputManager = scalarOutputManagers.erase(scalarOutputManager);
    }
}

void cMultipleOutputScalarManager::startRun()
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->startRun();
    }
}

void cMultipleOutputScalarManager::endRun()
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->endRun();
    }
}

void cMultipleOutputScalarManager::recordScalar(omnetpp::cComponent *component, const char *name, double value,
        omnetpp::opp_string_map *attributes)
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->recordScalar(component, name, value, attributes);
    }
}

void cMultipleOutputScalarManager::recordStatistic(omnetpp::cComponent *component, const char *name,
        omnetpp::cStatistic *statistic, omnetpp::opp_string_map *attributes)
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->recordStatistic(component, name, statistic, attributes);
    }
}

void cMultipleOutputScalarManager::flush()
{
    for (std::vector<omnetpp::cIOutputScalarManager*>::const_iterator scalarOutputManager =
            scalarOutputManagers.begin(); scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->flush();
    }
}

const char *cMultipleOutputScalarManager::getFileName() const
{
    return "undefined due to cMultipleOutputVectorManager";
}
