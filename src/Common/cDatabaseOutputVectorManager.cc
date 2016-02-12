#include "cDatabaseOutputVectorManager.h"

void *cDatabaseOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();
    vp->id = -1; // we'll get it from the database
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;
    vp->enabled = true;
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
