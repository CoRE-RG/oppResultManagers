#ifndef GCTAEVENTLOGMANAGER_H
#define GCTAEVENTLOGMANAGER_H

#include "omnetpp.h"

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

        virtual void configure() override;
        virtual void open() override {}
        virtual void startRun() override;
        virtual void endRun(bool isError, int resultCode, const char *message) override;

        virtual bool hasRecordingIntervals() const override;
        virtual void clearRecordingIntervals() override;

        virtual void recordSimulation() override;

        virtual void flush() override;

        /** @name Functions called from cEnvir's similar functions */
        //@{
        virtual void simulationEvent(omnetpp::cEvent *event) override;
        virtual void bubble(omnetpp::cComponent *component, const char *text) override {}
        virtual void messageScheduled(omnetpp::cMessage *msg) override {}
        virtual void messageCancelled(omnetpp::cMessage *msg) override {}
        virtual void beginSend(omnetpp::cMessage *msg) override;
        virtual void messageSendDirect(omnetpp::cMessage *msg, omnetpp::cGate *toGate,
                simtime_t propagationDelay, simtime_t transmissionDelay) override {}
        virtual void messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate) override {}
        virtual void messageSendHop(omnetpp::cMessage *msg, omnetpp::cGate *srcGate,
                simtime_t propagationDelay, simtime_t transmissionDelay) override {}
        virtual void endSend(omnetpp::cMessage *msg) override {}
        virtual void messageCreated(omnetpp::cMessage *msg) override {}
        virtual void messageCloned(omnetpp::cMessage *msg, omnetpp::cMessage *clone) override {}
        virtual void messageDeleted(omnetpp::cMessage *msg) override {}
        virtual void moduleReparented(omnetpp::cModule *module, omnetpp::cModule *oldparent, int oldId) override {}
        virtual void componentMethodBegin(omnetpp::cComponent *from, omnetpp::cComponent *to, const char *methodFmt, va_list va) override {}
        virtual void componentMethodEnd() override {}
        virtual void moduleCreated(omnetpp::cModule *newmodule) override {}
        virtual void moduleDeleted(omnetpp::cModule *module) override {}
        virtual void gateCreated(omnetpp::cGate *newgate) override {}
        virtual void gateDeleted(omnetpp::cGate *gate) override {}
        virtual void connectionCreated(omnetpp::cGate *srcgate) override;
        virtual void connectionDeleted(omnetpp::cGate *srcgate) override {}
        virtual void displayStringChanged(omnetpp::cComponent *component) override {}
        virtual void logLine(const char *prefix, const char *line, int lineLength) override {}
        //@}

};

#endif
