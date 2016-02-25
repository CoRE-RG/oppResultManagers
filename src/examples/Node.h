#ifndef OPPRESULTMANAGERS_NODE_H_
#define OPPRESULTMANAGERS_NODE_H_

#include <omnetpp.h>

class Node : public omnetpp::cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

  protected:
          /**
           * Signal that is emitted every time a message was received.
           */
          static omnetpp::simsignal_t rxMessageAgeSignal;
};

#endif
