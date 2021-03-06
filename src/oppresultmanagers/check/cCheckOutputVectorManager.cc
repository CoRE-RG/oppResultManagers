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

#include "oppresultmanagers/check/cCheckOutputVectorManager.h"

#include "oppresultmanagers/utilities/HelperFunctions.h"
#include "oppresultmanagers/utilities/fileutil.h"

#include <regex>
#include <algorithm>
#include <fstream>

Register_Class(cCheckOutputVectorManager);

Register_PerRunConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_CONFIGFILE, "checkoutputvectormanager-configfile",
        CFG_FILENAME, "constraints.xml", "TBD");
Register_PerRunConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_ENDSIM, "checkoutputvectormanager-endsimulation", CFG_BOOL,
        "false", "TBD");
Register_PerRunConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_REPORT, "checkoutputvectormanager-report", CFG_BOOL, "true",
        "TBD");
Register_PerRunConfigOption(CFGID_CHECKOUTPUTVECTORMANAGER_REPORTFILE, "checkoutputvectormanager-reportfile",
        CFG_FILENAME, "${resultdir}/${configname}-${runnumber}-constraintReport.csv", "TBD");

cCheckOutputVectorManager::cCheckOutputVectorManager()
{
    xmlConfiguration = nullptr;
    endSimulation = false;
    report = true;
    reportFilename = "";
}

cCheckOutputVectorManager::~cCheckOutputVectorManager()
{
    vectordata.clear();
}

void cCheckOutputVectorManager::startRun()
{
    xmlConfiguration = omnetpp::getEnvir()->getXMLDocument(
                omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_CHECKOUTPUTVECTORMANAGER_CONFIGFILE).c_str(),
                "/constraints");
    endSimulation = omnetpp::getEnvir()->getConfig()->getAsBool(CFGID_CHECKOUTPUTVECTORMANAGER_ENDSIM);
    report = omnetpp::getEnvir()->getConfig()->getAsBool(CFGID_CHECKOUTPUTVECTORMANAGER_REPORT);
    reportFilename = omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_CHECKOUTPUTVECTORMANAGER_REPORTFILE);
}

void cCheckOutputVectorManager::endRun()
{
    if (report)
    {
        outputReport();
    }
}

void *cCheckOutputVectorManager::registerVector(const char *modulename, const char *vectorname)
{
    sVectorData *vp = new sVectorData();

    vp->moduleName = modulename;
    vp->vectorName = vectorname;

    omnetpp::cXMLElementList constraints = xmlConfiguration->getChildrenByTagName("constraint");
    for (omnetpp::cXMLElementList::iterator constraint = constraints.begin(); constraint != constraints.end();
            ++constraint)
    {
        double factor = 1;
        if ((*constraint)->getAttribute("unit"))
        {
            if (0 == strcmp((*constraint)->getAttribute("unit"), "s"))
            {
                factor = 1;
            }
            else if (0 == strcmp((*constraint)->getAttribute("unit"), "ms"))
            {
                factor = 0.001;
            }
            else if (0 == strcmp((*constraint)->getAttribute("unit"), "us"))
            {
                factor = 0.000001;
            }
            else if (0 == strcmp((*constraint)->getAttribute("unit"), "ns"))
            {
                factor = 0.000000001;
            }
            else if (0 == strcmp((*constraint)->getAttribute("unit"), "ps"))
            {
                factor = 0.000000000001;
            }
        }

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
                omnetpp::cXMLElementList constraintsValues = (*constraint)->getChildren();
                for (omnetpp::cXMLElementList::iterator constraintsValue = constraintsValues.begin();
                        constraintsValue != constraintsValues.end(); ++constraintsValue)
                {
                    Constraint *newConstraint = nullptr;
                    if (0 == strcmp((*constraintsValue)->getTagName(), "min"))
                    {
                        newConstraint = new Constraint(Constraint::min);
                    }
                    else if (0 == strcmp((*constraintsValue)->getTagName(), "max"))
                    {
                        newConstraint = new Constraint(Constraint::max);
                    }
                    else if (0 == strcmp((*constraintsValue)->getTagName(), "sum_max"))
                    {
                        newConstraint = new SumConstraint(Constraint::sum_max);
                    }
                    else if (0 == strcmp((*constraintsValue)->getTagName(), "interval_min"))
                    {
                        newConstraint = new IntervalConstraint(Constraint::interval_min);
                    }
                    else if (0 == strcmp((*constraintsValue)->getTagName(), "interval_max"))
                    {
                        newConstraint = new IntervalConstraint(Constraint::interval_max);
                    }
                    else if ((0 == strcmp((*constraintsValue)->getTagName(), "avg_min"))
                            || (0 == strcmp((*constraintsValue)->getTagName(), "avg_max")))
                    {
                        const char* samples_string = (*constraintsValue)->getAttribute("samples");
                        if (!samples_string)
                        {
                            throw omnetpp::cRuntimeError("average constraint requires attribute samples");
                        }
                        size_t samples = strtoul(samples_string, nullptr, 0);
                        if (samples == 0)
                        {
                            throw omnetpp::cRuntimeError("samples attribute must be larger than 0");
                        }
                        else if (samples == 1)
                        {
                            throw omnetpp::cRuntimeError(
                                    "samples attribute must be larger than 1, if you want 1 sample, use min/max constraint instead");
                        }
                        if (0 == strcmp((*constraintsValue)->getTagName(), "avg_min"))
                        {
                            newConstraint = new AverageConstraint(Constraint::avg_min, samples);
                        }
                        else if (0 == strcmp((*constraintsValue)->getTagName(), "avg_max"))
                        {
                            newConstraint = new AverageConstraint(Constraint::avg_max, samples);
                        }
                    }
                    else
                    {
                        EV_ERROR << "Did not register constraint for module: " << modulename << " vector: "
                                << vectorname << " type was unknown!" << std::endl;
                        throw omnetpp::cRuntimeError("Unknown constraint type %s", (*constraintsValue)->getTagName());
                    }
                    double value = atof((*constraintsValue)->getNodeValue());
                    value *= factor;
                    newConstraint->value = value;
                    vp->constraints.push_back(newConstraint);
                    EV_INFO << "Registered constraint for module: " << modulename << " vector: " << vectorname
                            << " type: " << (*constraintsValue)->getTagName() << std::endl;
                }
            }
        }
    }
    vectordata.push_back(vp);
    return vp;
}
void cCheckOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);
    std::vector<sVectorData*>::iterator it = std::find(vectordata.begin(), vectordata.end(), vp);
    if (it != vectordata.end())
    {
        vectordata.erase(it);
    }
    delete vp;
}

void cCheckOutputVectorManager::setVectorAttribute(__attribute__((__unused__)) void *vectorhandle,
        __attribute__((__unused__)) const char *name, __attribute__((__unused__)) const char *value)
{
}

bool cCheckOutputVectorManager::record(void *vectorhandle, omnetpp::simtime_t t, double value)
{
    sVectorData *vp = static_cast<sVectorData *>(vectorhandle);

    for (std::vector<Constraint*>::iterator constraint = vp->constraints.begin(); constraint != vp->constraints.end();
            ++constraint)
    {
        double workValue = value;
        //Sum constraints require value to be the sum:
        if ((*constraint)->type == Constraint::sum_max)
        {
            SumConstraint *sconstraint = static_cast<SumConstraint*>(*constraint);
            sconstraint->sum += workValue;
            workValue = sconstraint->sum;
        }
        //Avg constraints require value to be the sum:
        else if (((*constraint)->type == Constraint::avg_min) || ((*constraint)->type == Constraint::avg_max))
        {
            AverageConstraint *aconstraint = static_cast<AverageConstraint*>(*constraint);
            aconstraint->samples[aconstraint->samplesPos] = workValue;
            aconstraint->samplesPos++;
            aconstraint->samplesPos %= aconstraint->requiredSamples;

            if (aconstraint->noSamples < aconstraint->requiredSamples)
            {
                aconstraint->noSamples++;
            }

            //not yet enough samples
            if (aconstraint->noSamples < aconstraint->requiredSamples)
            {
                aconstraint->noSamples++;
                continue;
            }
            else
            {
                workValue = 0;
                for (size_t i = 0; i < aconstraint->requiredSamples; i++)
                {
                    workValue += aconstraint->samples[i];
                }
                workValue /= static_cast<double>(aconstraint->noSamples);
            }
        }
        //Interval constraints require value to be the interval:
        else if (((*constraint)->type == Constraint::interval_min) || ((*constraint)->type == Constraint::interval_max))
        {
            IntervalConstraint *iconstraint = static_cast<IntervalConstraint*>(*constraint);
            if (iconstraint->last != 0)
            {
                workValue = (t - iconstraint->last).dbl();
                iconstraint->last = t;
            }
            else
            {
                iconstraint->last = t;
                continue;
            }
        }

        bool violationStart = false;
        bool violationEnd = false;
        bool inViolation = ((*constraint)->intervallStart != 0);

        if (!inViolation)
        {
            if ((((*constraint)->type == Constraint::min) || ((*constraint)->type == Constraint::interval_min)
                    || ((*constraint)->type == Constraint::avg_min)) && (workValue < (*constraint)->value))
            {
                violationStart = true;
            }
            else if ((((*constraint)->type == Constraint::max) || ((*constraint)->type == Constraint::interval_max)
                    || ((*constraint)->type == Constraint::sum_max) || ((*constraint)->type == Constraint::avg_max))
                    && (workValue > (*constraint)->value))
            {
                violationStart = true;
            }

            //This constraint was violated
            if (violationStart)
            {
                (*constraint)->violation = true;
                (*constraint)->intervallStart = t;
            }
        }
        else
        {
            if ((((*constraint)->type == Constraint::min) || ((*constraint)->type == Constraint::interval_min)
                    || ((*constraint)->type == Constraint::avg_min)) && (workValue > (*constraint)->value))
            {
                violationEnd = true;
            }
            else if ((((*constraint)->type == Constraint::max) || ((*constraint)->type == Constraint::interval_max)
                    || ((*constraint)->type == Constraint::sum_max) || ((*constraint)->type == Constraint::avg_max))
                    && (workValue < (*constraint)->value))
            {
                violationEnd = true;
            }

            if (violationEnd)
            {
                (*constraint)->intervals.push_back(std::pair<omnetpp::simtime_t, omnetpp::simtime_t>((*constraint)->intervallStart, t));
                (*constraint)->intervallStart = 0;
            }
        }

        if (endSimulation && violationStart)
        {
            if (report)
            {
                outputReport();
            }
            throw omnetpp::cRuntimeError("Simulation ended due to constraint violation.");
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

void cCheckOutputVectorManager::outputReport()
{
    std::string fileType = reportFilename.substr(std::max<size_t>(4, reportFilename.size()) - 4);
    if (fileType == ".csv")
    {
        removeFile(reportFilename.c_str(), "old eventlog file");
        mkPath(directoryOf(reportFilename.c_str()).c_str());

        std::fstream file;
        file.open(reportFilename, std::fstream::out);
        file << "Module,Vector,Constraint Type,Constraint Value" << std::endl;
        for (std::vector<sVectorData*>::const_iterator vectorDataEntry = vectordata.begin();
                vectorDataEntry != vectordata.end(); ++vectorDataEntry)
        {
            for (std::vector<Constraint*>::const_iterator constraint = (*vectorDataEntry)->constraints.begin();
                    constraint != (*vectorDataEntry)->constraints.end(); ++constraint)
            {
                if ((*constraint)->violation)
                {
                    file << (*vectorDataEntry)->moduleName << "," << (*vectorDataEntry)->vectorName << ",";
                    switch ((*constraint)->type)
                    {
                        case Constraint::min:
                            file << "min";
                            break;
                        case Constraint::max:
                            file << "max";
                            break;
                        case Constraint::avg_min:
                            file << "avg_min";
                            break;
                        case Constraint::avg_max:
                            file << "avg_max";
                            break;
                        case Constraint::interval_min:
                            file << "interval_min";
                            break;
                        case Constraint::interval_max:
                            file << "interval_max";
                            break;
                        case Constraint::sum_max:
                            file << "sum_max";
                            break;
                    }
                    file << "," << (*constraint)->value << ",";
                    for (std::vector<std::pair<omnetpp::simtime_t, omnetpp::simtime_t>>::const_iterator interval =
                            (*constraint)->intervals.begin(); interval != (*constraint)->intervals.end(); ++interval)
                    {
                        file << (*interval).first << ".." << (*interval).second << " ";
                    }
                    if ((*constraint)->intervallStart != 0)
                    {
                        file << (*constraint)->intervallStart << ".." << " ";
                    }
                    file << std::endl;
                }
            }
        }
        file.close();
    }
}
