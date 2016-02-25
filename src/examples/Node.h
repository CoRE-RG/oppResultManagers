#ifndef OPPRESULTMANAGERS_NODE_H_
#define OPPRESULTMANAGERS_NODE_H_

#include <omnetpp.h>

class Node : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  protected:
          /**
           * Signal that is emitted every time a message was received.
           */
          static simsignal_t rxMessageAgeSignal;
};

#endif
