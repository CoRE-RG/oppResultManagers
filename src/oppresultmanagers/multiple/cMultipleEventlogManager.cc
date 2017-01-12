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

#include "oppresultmanagers/multiple/cMultipleEventlogManager.h"

Register_Class(cMultipleEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOGMANAGER_CLASSES, "eventlogmanager-classes", CFG_STRING, "\"\"",
        "List of EventlogManager Classes (comma or space separated)");

cMultipleEventlogManager::cMultipleEventlogManager()
{
    std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_EVENTLOGMANAGER_CLASSES);

    std::vector<std::string> managerClasses = omnetpp::cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator managerClass = managerClasses.begin();
            managerClass != managerClasses.end(); ++managerClass)
    {
        omnetpp::cObject *eventlogmgr_tmp = omnetpp::createOne((*managerClass).c_str());
        if (omnetpp::cIEventlogManager * eventlogManager = dynamic_cast<omnetpp::cIEventlogManager *>(eventlogmgr_tmp))
        {
            eventlogManagers.push_back(eventlogManager);
        }
        else
        {
            throw omnetpp::cRuntimeError("Class \"%s\" is not subclassed from cIEventlogManager",
                    (*managerClass).c_str());
        }
    }
}

cMultipleEventlogManager::~cMultipleEventlogManager()
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end();)
    {
        eventlogManager = eventlogManagers.erase(eventlogManager);
    }
}


void cMultipleEventlogManager::startRecording()
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->startRecording();
    }
}

void cMultipleEventlogManager::stopRecording()
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->stopRecording();
    }
}

void cMultipleEventlogManager::flush()
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->flush();
    }
}

#if OMNETPP_VERSION >= 0x0501
const char *cMultipleEventlogManager::getFileName() const
{
    if (eventlogManagers.size() > 0)
    {
        return eventlogManagers.front()->getFileName();
    }
    return nullptr;
}
#endif

void cMultipleEventlogManager::simulationEvent(omnetpp::cEvent *event)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->simulationEvent(event);
    }
}

void cMultipleEventlogManager::bubble(omnetpp::cComponent *component, const char *text)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->bubble(component, text);
    }
}

void cMultipleEventlogManager::messageScheduled(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageScheduled(msg);
    }
}

void cMultipleEventlogManager::messageCancelled(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageCancelled(msg);
    }
}

void cMultipleEventlogManager::beginSend(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->beginSend(msg);
    }
}

void cMultipleEventlogManager::messageSendDirect(omnetpp::cMessage *msg, omnetpp::cGate *toGate, omnetpp::simtime_t propagationDelay,
        omnetpp::simtime_t transmissionDelay)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageSendDirect(msg, toGate, propagationDelay, transmissionDelay);
    }
}

void cMultipleEventlogManager::messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageSendHop(msg, srcGate);
    }
}

#if OMNETPP_VERSION >= 0x0501
void cMultipleEventlogManager::messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate, omnetpp::simtime_t propagationDelay,
        omnetpp::simtime_t transmissionDelay, bool discard)
#else
void cMultipleEventlogManager::messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate, omnetpp::simtime_t propagationDelay,
        omnetpp::simtime_t transmissionDelay)
#endif
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
#if OMNETPP_VERSION >= 0x0501
        (*eventlogManager)->messageSendHop(msg, srcGate, propagationDelay, transmissionDelay, discard);
#else
        (*eventlogManager)->messageSendHop(msg, srcGate, propagationDelay, transmissionDelay);
#endif
    }
}

void cMultipleEventlogManager::endSend(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->endSend(msg);
    }
}

void cMultipleEventlogManager::messageCreated(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageCreated(msg);
    }
}

void cMultipleEventlogManager::messageCloned(omnetpp::cMessage *msg, omnetpp::cMessage *clone)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageCloned(msg, clone);
    }
}

void cMultipleEventlogManager::messageDeleted(omnetpp::cMessage *msg)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->messageDeleted(msg);
    }
}

void cMultipleEventlogManager::moduleReparented(omnetpp::cModule *module, omnetpp::cModule *oldparent, int oldId)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->moduleReparented(module, oldparent, oldId);
    }
}

void cMultipleEventlogManager::componentMethodBegin(omnetpp::cComponent *from, omnetpp::cComponent *to, const char *methodFmt, va_list va)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->componentMethodBegin(from, to, methodFmt, va);
    }
}

void cMultipleEventlogManager::componentMethodEnd()
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->componentMethodEnd();
    }
}

void cMultipleEventlogManager::moduleCreated(omnetpp::cModule *newmodule)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->moduleCreated(newmodule);
    }
}

void cMultipleEventlogManager::moduleDeleted(omnetpp::cModule *module)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->moduleDeleted(module);
    }
}

void cMultipleEventlogManager::gateCreated(omnetpp::cGate *newgate)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->gateCreated(newgate);
    }
}

void cMultipleEventlogManager::gateDeleted(omnetpp::cGate *gate)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->gateDeleted(gate);
    }
}

void cMultipleEventlogManager::connectionCreated(omnetpp::cGate *srcgate)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->connectionCreated(srcgate);
    }
}

void cMultipleEventlogManager::connectionDeleted(omnetpp::cGate *srcgate)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->connectionDeleted(srcgate);
    }
}

void cMultipleEventlogManager::displayStringChanged(omnetpp::cComponent *component)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->displayStringChanged(component);
    }
}

void cMultipleEventlogManager::logLine(const char *prefix, const char *line, int lineLength)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->logLine(prefix, line, lineLength);
    }
}

void cMultipleEventlogManager::stoppedWithException(bool isError, int resultCode, const char *message)
{
    for (std::vector<omnetpp::cIEventlogManager*>::const_iterator eventlogManager = eventlogManagers.begin();
            eventlogManager != eventlogManagers.end(); ++eventlogManager)
    {
        (*eventlogManager)->stoppedWithException(isError, resultCode, message);
    }
}


