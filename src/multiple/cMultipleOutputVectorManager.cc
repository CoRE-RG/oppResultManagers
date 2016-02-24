#include "cMultipleOutputVectorManager.h"

Register_Class(cMultipleOutputVectorManager);

Register_PerRunConfigOption(CFGID_OUTPUTVECTORMANAGER_CLASSES, "outputvectormanager-classes", CFG_STRING, "\"\"",
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
    sVectorData *vp = (sVectorData *) vectorhandle;
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
    sVectorData *vp = (sVectorData *) vectorhandle;
    std::vector<omnetpp::cIOutputVectorManager*>::const_iterator vectorOutputManager = vectorOutputManagers.begin();
    std::vector<void*>::const_iterator vectorOutputManagerHandle = vp->vectorhandles.begin();
    while ((vectorOutputManager != vectorOutputManagers.end()) && (vectorOutputManagerHandle != vp->vectorhandles.end()))
    {
        (*vectorOutputManager)->setVectorAttribute((*vectorOutputManagerHandle), name, value);

        ++vectorOutputManager;
        ++vectorOutputManagerHandle;
    }
}
bool cMultipleOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    bool returnValue = false;

    sVectorData *vp = (sVectorData *) vectorhandle;
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
