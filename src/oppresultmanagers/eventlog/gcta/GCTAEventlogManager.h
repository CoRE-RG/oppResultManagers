#ifndef GCTAEVENTLOGMANAGER_H
#define GCTAEVENTLOGMANAGER_H

#include "omnetpp.h"

extern omnetpp::cConfigOption *CFGID_EVENTLOG_TLOG_FILE;

/**
 * Responsible for writing the eventlog file.
 */
class GCTAEventlogManager : public omnetpp::cIEventlogManager
{
    private:
        omnetpp::opp_string filename;
        FILE *feventlog;
        bool recordEventlog;

    public:
        GCTAEventlogManager();
        virtual ~GCTAEventlogManager();

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
                __attribute__((__unused__)) omnetpp::simtime_t propagationDelay, __attribute__((__unused__)) omnetpp::simtime_t transmissionDelay) override {}
        virtual void messageSendHop(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cGate *srcGate) override {}
        virtual void messageSendHop(__attribute__((__unused__)) omnetpp::cMessage *msg, __attribute__((__unused__)) omnetpp::cGate *srcGate,
                __attribute__((__unused__)) omnetpp::simtime_t propagationDelay, __attribute__((__unused__)) omnetpp::simtime_t transmissionDelay) override {}
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
        virtual void connectionCreated(__attribute__((__unused__)) omnetpp::cGate *srcgate) override;
        virtual void connectionDeleted(__attribute__((__unused__)) omnetpp::cGate *srcgate) override {}
        virtual void displayStringChanged(__attribute__((__unused__)) omnetpp::cComponent *component) override {}
        virtual void logLine(__attribute__((__unused__)) const char *prefix, __attribute__((__unused__)) const char *line, __attribute__((__unused__)) int lineLength) override {}
        virtual void stoppedWithException(__attribute__((__unused__)) bool isError,__attribute__((__unused__))  int resultCode,__attribute__((__unused__)) const char *message) override {}
        //@}

};

#endif
