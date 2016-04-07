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

#ifndef OPPRESULTMANAGERS_PCAPEVENTLOGMANAGER_H
#define OPPRESULTMANAGERS_PCAPEVENTLOGMANAGER_H

#include "omnetpp.h"

#include "oppresultmanagers/base/oppResultManagers_Defs.h"
#include "oppresultmanagers/eventlog/pcapng/PCAPNGWriter.h"

extern omnetpp::cConfigOption *CFGID_EVENTLOG_PCAPNG_FILE;
extern omnetpp::cConfigOption *CFGID_EVENTLOG_PCAPNG_INTERFACES;
extern omnetpp::cConfigOption *CFGID_EVENTLOG_PCAPNG_CAPTURELENGTH;

/**
 * Responsible for writing the eventlog file.
 */
class PCAPNGEventlogManager : public omnetpp::cIEventlogManager
{
    private:
        omnetpp::opp_string filename;
        bool recordEventlog;
        void* buffer;
        PCAPNGWriter *pcapwriter;
        bool recordingStarted = false;
        std::map<omnetpp::cModule*,size_t> interfaceMap;
        size_t capture_length;

    public:
        PCAPNGEventlogManager();
        virtual ~PCAPNGEventlogManager();

        virtual void startRecording() override;
        virtual void stopRecording() override;

        virtual void flush() override;

        /** @name Functions called from cEnvir's similar functions */
        //@{
        virtual void simulationEvent(omnetpp::cEvent *event) override;
        virtual void bubble(__attribute__((__unused__)) omnetpp::cComponent *component, __attribute__((__unused__)) const char *text) override {}
        virtual void messageScheduled(__attribute__((__unused__)) omnetpp::cMessage *msg) override {}
        virtual void messageCancelled(__attribute__((__unused__)) omnetpp::cMessage *msg) override {}
        virtual void beginSend(__attribute__((__unused__)) omnetpp::cMessage *msg) override;
        virtual void messageSendDirect(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cGate *toGate,
                __attribute__((__unused__)) simtime_t propagationDelay, __attribute__((__unused__)) simtime_t transmissionDelay) override {}
        virtual void messageSendHop(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cGate *srcGate) override {}
        virtual void messageSendHop(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cGate *srcGate,
                __attribute__((__unused__)) simtime_t propagationDelay, __attribute__((__unused__)) simtime_t transmissionDelay) override {}
        virtual void endSend(__attribute__((__unused__)) omnetpp::cMessage *msg) override {}
        virtual void messageCreated(__attribute__((__unused__)) omnetpp::cMessage *msg) override {}
        virtual void messageCloned(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cMessage *clone) override {}
        virtual void messageDeleted(__attribute__((__unused__)) omnetpp::cMessage *msg) override {}
        virtual void moduleReparented(__attribute__((__unused__)) omnetpp::cModule *module,__attribute__((__unused__)) omnetpp::cModule *oldparent, __attribute__((__unused__))  int oldId) override {}
        virtual void componentMethodBegin(__attribute__((__unused__)) omnetpp::cComponent *from, __attribute__((__unused__)) omnetpp::cComponent *to, __attribute__((__unused__)) const char *methodFmt, __attribute__((__unused__)) va_list va) override {}
        virtual void componentMethodEnd() override {}
        virtual void moduleCreated(__attribute__((__unused__)) omnetpp::cModule *newmodule) override {}
        virtual void moduleDeleted(__attribute__((__unused__)) omnetpp::cModule *module) override {}
        virtual void gateCreated(__attribute__((__unused__)) omnetpp::cGate *newgate) override {}
        virtual void gateDeleted(__attribute__((__unused__)) omnetpp::cGate *gate) override {}
        virtual void connectionCreated(__attribute__((__unused__)) omnetpp::cGate *srcgate) override {}
        virtual void connectionDeleted(__attribute__((__unused__)) omnetpp::cGate *srcgate) override {}
        virtual void displayStringChanged(__attribute__((__unused__)) omnetpp::cComponent *component) override {}
        virtual void logLine(__attribute__((__unused__)) const char *prefix, __attribute__((__unused__)) const char *line, __attribute__((__unused__)) int lineLength) override {}
        virtual void stoppedWithException(__attribute__((__unused__)) bool isError, __attribute__((__unused__)) int resultCode, __attribute__((__unused__)) const char *message) override {}
        //@}

};

#endif
