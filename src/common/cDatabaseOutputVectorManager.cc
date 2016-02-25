#include "cDatabaseOutputVectorManager.h"

#include "HelperFunctions.h"

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

    vp->enabled = ev.getConfig()->getAsBool(CFGID_VECTOR_RECORDING, true);
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
