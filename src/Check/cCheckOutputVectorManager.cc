#include "cCheckOutputVectorManager.h"

Register_Class(cCheckOutputVectorManager);

Register_GlobalConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_CONFIGFILE, "checkoutputvectormanager-configfile",
        CFG_FILENAME, "\"\"", "TBD");

cCheckOutputVectorManager::cCheckOutputVectorManager()
{
}

cCheckOutputVectorManager::~cCheckOutputVectorManager()
{
}

void cCheckOutputVectorManager::startRun()
{
}

void cCheckOutputVectorManager::endRun()
{
}

void *cCheckOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();
    return vp;
}
void cCheckOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *) vectorhandle;
    delete vp;
}
void cCheckOutputVectorManager::setVectorAttribute(void *vectorhandle, const char *name, const char *value)
{
}
bool cCheckOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    static int test = 0;
    test++;
    if(test==100)
        throw cTerminationException("module lala violates range check");
    return true;
}

void cCheckOutputVectorManager::flush()
{
}

const char *cCheckOutputVectorManager::getFileName() const
{
    return nullptr;
}
