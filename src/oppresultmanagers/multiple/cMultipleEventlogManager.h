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

#ifndef CMULTIPLEEVENTLOGMANAGER_H
#define CMULTIPLEEVENTLOGMANAGER_H

#include <omnetpp.h>

//Std
#include <vector>

extern omnetpp::cConfigOption *CFGID_EVENTLOGMANAGER_CLASSES;

class cMultipleEventlogManager : public omnetpp::cIEventlogManager
{
    protected:
        std::vector<omnetpp::cIEventlogManager *> eventlogManagers;
    public:
        /** @name Constructor, destructor */
        //@{

        /**
         * Constructor.
         */
        explicit cMultipleEventlogManager();

        /**
         * Destructor.
         */
        virtual ~cMultipleEventlogManager();
        //@}

        /** @name Controlling the beginning and end of collecting data. */
        //@{

        /**
         * (Re)starts eventlog recording. Whether eventlog recording is enabled by
         * default depends on the eventlog manager (e.g. omnetpp.ini configuration
         * options). This function may be called several times (from the user
         * interface) during the simulation, but only between events.
         * The eventlog manager is expected to produce output starting from the
         * next event.
         */
        virtual void startRecording();

        /**
         * Temporarily stops eventlog recording. See startRecording() for more details.
         */
        virtual void stopRecording();

        /**
         * Forces writing out all buffered output.
         */
        virtual void flush();
        //@}

        /** @name Functions called from cEnvir's similar functions */
        //@{
        virtual void simulationEvent(omnetpp::cEvent *event);
        virtual void bubble(omnetpp::cComponent *component, const char *text);
        virtual void messageScheduled(omnetpp::cMessage *msg);
        virtual void messageCancelled(omnetpp::cMessage *msg);
        virtual void beginSend(omnetpp::cMessage *msg);
        virtual void messageSendDirect(omnetpp::cMessage *msg, omnetpp::cGate *toGate, simtime_t propagationDelay, simtime_t transmissionDelay);
        virtual void messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate);
        virtual void messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate, simtime_t propagationDelay, simtime_t transmissionDelay);
        virtual void endSend(omnetpp::cMessage *msg);
        virtual void messageCreated(omnetpp::cMessage *msg);
        virtual void messageCloned(omnetpp::cMessage *msg, omnetpp::cMessage *clone);
        virtual void messageDeleted(omnetpp::cMessage *msg);
        virtual void moduleReparented(omnetpp::cModule *module, omnetpp::cModule *oldparent, int oldId);
        virtual void componentMethodBegin(omnetpp::cComponent *from, omnetpp::cComponent *to, const char *methodFmt, va_list va);
        virtual void componentMethodEnd();
        virtual void moduleCreated(omnetpp::cModule *newmodule);
        virtual void moduleDeleted(omnetpp::cModule *module);
        virtual void gateCreated(omnetpp::cGate *newgate);
        virtual void gateDeleted(omnetpp::cGate *gate);
        virtual void connectionCreated(omnetpp::cGate *srcgate);
        virtual void connectionDeleted(omnetpp::cGate *srcgate);
        virtual void displayStringChanged(omnetpp::cComponent *component);
        virtual void logLine(const char *prefix, const char *line, int lineLength);
        virtual void stoppedWithException(bool isError, int resultCode, const char *message);
        //@}
};

#endif

