#include "cCheckOutputVectorManager.h"

#include <regex>

Register_Class(cCheckOutputVectorManager);

Register_GlobalConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_CONFIGFILE, "checkoutputvectormanager-configfile", CFG_PATH,
        "\"\"", "TBD");

cCheckOutputVectorManager::cCheckOutputVectorManager()
{
    xmlConfiguration = simulation.getEnvir()->getXMLDocument(
    ev.getConfig()->getAsPath(CFGID_CHECKOUTPUTVECTORMANAGER_CONFIGFILE).c_str(), "/constraints");
}

cCheckOutputVectorManager::~cCheckOutputVectorManager()
{
}

void cCheckOutputVectorManager::startRun()
{

}

void cCheckOutputVectorManager::endRun()
{
    //Write violation
}

void *cCheckOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();

    cXMLElementList constraints = xmlConfiguration->getChildrenByTagName("constraint");
    for (cXMLElementList::iterator constraint = constraints.begin(); constraint != constraints.end(); ++constraint)
    {
        //Check module for this vector?
        if ((*constraint)->getAttribute("module"))
        {
            if ((*constraint)->getAttribute("moduleIsRegex")
                    && 0 == strcmp((*constraint)->getAttribute("moduleIsRegex"), "true"))
            {
                std::regex moduleRegex((*constraint)->getAttribute("module"));
                if (!std::regex_match(modulename, moduleRegex))
                {
                    continue;
                }
            }
            else
            {
                if (0 != strcmp((*constraint)->getAttribute("module"), modulename))
                {
                    continue;
                }
            }

            //Check this vector is ment?
            if ((*constraint)->getAttribute("name"))
            {
                if ((*constraint)->getAttribute("nameIsRegex")
                        && 0 == strcmp((*constraint)->getAttribute("nameIsRegex"), "true"))
                {
                    std::regex nameRegex((*constraint)->getAttribute("name"));
                    if (!std::regex_match(vectorname, nameRegex))
                    {
                        continue;
                    }
                }
                else
                {
                    if (0 != strcmp((*constraint)->getAttribute("name"), vectorname))
                    {
                        continue;
                    }
                }

                //
                cXMLElementList constraintsValues = (*constraint)->getChildren();
                for (cXMLElementList::iterator constraintsValue = constraintsValues.begin();
                        constraintsValue != constraintsValues.end(); ++constraintsValue)
                {
                    if (0 == strcmp((*constraintsValue)->getTagName(), "min"))
                    {
                        double min = atof((*constraintsValue)->getNodeValue());
                        if (min > vp->min)
                        {
                            vp->min = min;
                            ev << "Registered constraint for module: " << modulename << " vector: " << vectorname
                                    << " minimum: " << min << endl;
                        }
                        vp->minEnabled = true;
                    }
                    else if (0 == strcmp((*constraintsValue)->getTagName(), "max"))
                    {
                        double max = atof((*constraintsValue)->getNodeValue());
                        if (max < vp->max)
                        {
                            vp->max = max;
                            ev << "Registered constraint for module: " << modulename << " vector: " << vectorname
                                    << " maximum: " << max << endl;
                        }
                        vp->maxEnabled = true;
                    }
                }
            }
        }
    }
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
    sVectorData *vp = (sVectorData *) vectorhandle;

    if (vp->minEnabled)
    {
        if (value < vp->min)
        {
            throw cTerminationException("module lala violates range check");
        }
    }
    if (vp->maxEnabled)
    {
        if (value > vp->max)
        {
            throw cTerminationException("module lala violates range check");
        }
    }

    return true;
}

void cCheckOutputVectorManager::flush()
{
}

const char *cCheckOutputVectorManager::getFileName() const
{
    return nullptr;
}
