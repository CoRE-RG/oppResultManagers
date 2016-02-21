#include "cDatabaseOutputVectorManager.h"

#include "HelperFunctions.h"

void *cDatabaseOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    std::string vectorfullpath = std::string(modulename) + "." + vectorname;

    sVectorData *vp = new sVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;

    //TODO Work with extern instead?
    vp->enabled = omnetpp::cConfiguration::parseBool(
    omnetpp::getEnvir()->getConfig()->getPerObjectConfigEntry(vectorfullpath.c_str(), "vector-recording").getValue(), "true");

    const char *text = omnetpp::getEnvir()->getConfig()->getPerObjectConfigEntry(vectorfullpath.c_str(),
            "vector-recording-intervals").getValue();
    vp->intervals = parseIntervals(text);
    return vp;
}
void cDatabaseOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *) vectorhandle;
    delete vp;
}
void cDatabaseOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
    ASSERT(vectorhandle != NULL);
    sVectorData *vp = (sVectorData *) vectorhandle;
    vp->attributes[name] = value;
}
