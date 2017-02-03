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

#include "oppresultmanagers/multiple/cMultipleOutputVectorManager.h"

Register_Class(cMultipleOutputVectorManager);

Register_GlobalConfigOption(CFGID_OUTPUTVECTORMANAGER_CLASSES, "outputvectormanager-classes", CFG_STRING, "\"\"",
        "List of OutputVectorManager Classes (comma or space separated)");

cMultipleOutputVectorManager::cMultipleOutputVectorManager()
{
    std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_OUTPUTVECTORMANAGER_CLASSES);

    std::vector<std::string> managerClasses = omnetpp::cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator managerClass = managerClasses.begin();
            managerClass != managerClasses.end(); ++managerClass)
    {
        omnetpp::cObject *outvectormgr_tmp = omnetpp::createOne((*managerClass).c_str());
        if (omnetpp::cIOutputVectorManager * vectorManager =
                dynamic_cast<omnetpp::cIOutputVectorManager *>(outvectormgr_tmp))
        {
            vectorOutputManagers.push_back(vectorManager);
        }
        else
        {
            throw omnetpp::cRuntimeError("Class \"%s\" is not subclassed from cOutputVectorManager",
                    (*managerClass).c_str());
        }
    }
}

cMultipleOutputVectorManager::~cMultipleOutputVectorManager()
{
    for (std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager =
            vectorOutputManagers.begin(); vectorOutputManager != vectorOutputManagers.end();)
    {
        vectorOutputManager = vectorOutputManagers.erase(vectorOutputManager);
    }
}

void cMultipleOutputVectorManager::startRun()
{
    for (std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager =
            vectorOutputManagers.begin(); vectorOutputManager != vectorOutputManagers.end(); ++vectorOutputManager)
    {
        (*vectorOutputManager)->startRun();
    }
}

void cMultipleOutputVectorManager::endRun()
{
    for (std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager =
            vectorOutputManagers.begin(); vectorOutputManager != vectorOutputManagers.end(); ++vectorOutputManager)
    {
        (*vectorOutputManager)->endRun();
    }
}

void *cMultipleOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();
    for (std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager =
            vectorOutputManagers.begin(); vectorOutputManager != vectorOutputManagers.end(); ++vectorOutputManager)
    {
        void* vectorhandle = (*vectorOutputManager)->registerVector(modulename, vectorname);
        vp->vectorhandles.push_back(vectorhandle);
    }
    return vp;
}
void cMultipleOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager = vectorOutputManagers.begin();
    std::vector<void*>::const_iterator vectorOutputManagerHandle = vp->vectorhandles.begin();
    while ((vectorOutputManager != vectorOutputManagers.end()) && (vectorOutputManagerHandle != vp->vectorhandles.end()))
    {
        (*vectorOutputManager)->deregisterVector(*vectorOutputManagerHandle);

        ++vectorOutputManager;
        ++vectorOutputManagerHandle;
    }
    delete vp;
}
void cMultipleOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager = vectorOutputManagers.begin();
    std::vector<void*>::const_iterator vectorOutputManagerHandle = vp->vectorhandles.begin();
    while ((vectorOutputManager != vectorOutputManagers.end()) && (vectorOutputManagerHandle != vp->vectorhandles.end()))
    {
        (*vectorOutputManager)->setVectorAttribute((*vectorOutputManagerHandle), name, value);

        ++vectorOutputManager;
        ++vectorOutputManagerHandle;
    }
}
bool cMultipleOutputVectorManager::record(void *vectorhandle, omnetpp::simtime_t t, double value)
{
    bool returnValue = false;

    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager = vectorOutputManagers.begin();
    std::vector<void*>::const_iterator vectorOutputManagerHandle = vp->vectorhandles.begin();
    while ((vectorOutputManager != vectorOutputManagers.end()) && (vectorOutputManagerHandle != vp->vectorhandles.end()))
    {
        returnValue = (*vectorOutputManager)->record((*vectorOutputManagerHandle), t, value) || returnValue;
        ++vectorOutputManager;
        ++vectorOutputManagerHandle;
    }
    return returnValue;
}

void cMultipleOutputVectorManager::flush()
{
    for (std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager = vectorOutputManagers.begin();
            vectorOutputManager != vectorOutputManagers.end(); ++vectorOutputManager)
    {
        (*vectorOutputManager)->flush();
    }
}

const char *cMultipleOutputVectorManager::getFileName() const
{
    return "undefined due to cMultipleOutputVectorManager";
}
