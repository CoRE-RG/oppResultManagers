#include "cMultipleOutputScalarManager.h"

Register_Class(cMultipleOutputScalarManager);

Register_PerRunConfigOption(CFGID_OUTPUTSCALARMANAGER_CLASSES, "outputscalarmanager-classes", CFG_STRING, "\"\"",
        "List of OutputScalarManager Classes (comma or space separated)");

cMultipleOutputScalarManager::cMultipleOutputScalarManager()
{
    std::string cfgobj = ev.getConfig()->getAsString(CFGID_OUTPUTSCALARMANAGER_CLASSES);

    std::vector<std::string> managerClasses = cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator managerClass = managerClasses.begin();
            managerClass != managerClasses.end(); ++managerClass)
    {
        cObject *outscalarmgr_tmp = createOne((*managerClass).c_str());
        if (cOutputScalarManager * scalarManager = dynamic_cast<cOutputScalarManager *>(outscalarmgr_tmp))
        {
            scalarOutputManagers.push_back(scalarManager);
        }
        else
        {
            throw cRuntimeError("Class \"%s\" is not subclassed from cOutputScalarManager", (*managerClass).c_str());
        }
    }
}

cMultipleOutputScalarManager::~cMultipleOutputScalarManager()
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end();)
    {
        scalarOutputManager = scalarOutputManagers.erase(scalarOutputManager);
    }
}

void cMultipleOutputScalarManager::startRun()
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->startRun();
    }
}

void cMultipleOutputScalarManager::endRun()
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->endRun();
    }
}

void cMultipleOutputScalarManager::recordScalar(cComponent *component, const char *name, double value,
        opp_string_map *attributes)
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->recordScalar(component, name, value, attributes);
    }
}

void cMultipleOutputScalarManager::recordStatistic(cComponent *component, const char *name, cStatistic *statistic,
        opp_string_map *attributes)
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->recordStatistic(component, name, statistic, attributes);
    }
}

void cMultipleOutputScalarManager::flush()
{
    for (std::vector<cOutputScalarManager*>::const_iterator scalarOutputManager = scalarOutputManagers.begin();
            scalarOutputManager != scalarOutputManagers.end(); ++scalarOutputManager)
    {
        (*scalarOutputManager)->flush();
    }
}

const char *cMultipleOutputScalarManager::getFileName() const
{
    return "undefined due to cMultipleOutputVectorManager";
}
