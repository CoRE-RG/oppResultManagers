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

#include "oppresultmanagers/common/cDatabaseOutputVectorManager.h"

#include "oppresultmanagers/utilities/HelperFunctions.h"

extern cConfigOption* CFGID_VECTOR_RECORDING;
extern cConfigOption* CFGID_VECTOR_RECORDING_INTERVALS;

void *cDatabaseOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    std::string vectorfullpath = std::string(modulename) + "." + vectorname;

    sVectorData *vp = new sVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;

    vp->enabled = ev.getConfig()->getAsBool(vectorfullpath.c_str(), CFGID_VECTOR_RECORDING, true);
    const char *text = ev.getConfig()->getAsCustom(vectorfullpath.c_str(), CFGID_VECTOR_RECORDING_INTERVALS, "");
    vp->intervals = parseIntervals(text);
    return vp;
}
void cDatabaseOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    delete vp;
}
void cDatabaseOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    ASSERT(vectorhandle != nullptr);
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    vp->attributes[name] = value;
}
