#include "oppresultmanagers/eventlog/gcta/GCTAEventlogManager.h"

#include "oppresultmanagers/utilities/HelperFunctions.h"

Register_Class(GCTAEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOG_TLOG_FILE, "tlog-file", CFG_FILENAME,
        "${resultdir}/${configname}-${runnumber}.tlog", "Name of the GCTA timing log file to generate.");

GCTAEventlogManager::GCTAEventlogManager()
{
    recordEventlog = false;
    feventlog = nullptr;

    // main switch
    recordEventlog = omnetpp::cConfiguration::parseBool(
            omnetpp::getEnvir()->getConfig()->getConfigEntry("record-eventlog").getValue(), "false");

    // setup filename
    filename = omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_EVENTLOG_TLOG_FILE).c_str();

    omnetpp::common::removeFile(filename.c_str(), "old eventlog file");
    omnetpp::common::mkPath(omnetpp::common::directoryOf(filename.c_str()).c_str());
    FILE *out = fopen(filename.c_str(), "w");
    if (!out)
        throw omnetpp::cRuntimeError("Cannot open eventlog file `%s' for write", filename.c_str());
    ::printf("Recording eventlog to file `%s'...\n", filename.c_str());
    feventlog = out;
}

GCTAEventlogManager::~GCTAEventlogManager()
{
    if (feventlog)
    {
        fflush(feventlog);
        fclose(feventlog);

        feventlog = nullptr;
    }
}

void GCTAEventlogManager::startRecording()
{
}

void GCTAEventlogManager::stopRecording()
{
}

void GCTAEventlogManager::flush()
{
    if (recordEventlog)
        fflush(feventlog);
}

void GCTAEventlogManager::simulationEvent(__attribute__((__unused__))   omnetpp::cEvent *event)
{

}

void GCTAEventlogManager::beginSend(omnetpp::cMessage *msg)
{
    if (msg->isPacket() && recordEventlog)
    {

        omnetpp::cModule *mod = msg->getSenderModule();

        //Mac Module Arrive/Departure at Module
        if (mod && strcmp(mod->getFullName(), "mac") == 0)
        {
            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);
            const char * name = mod->getParentModule()->getParentModule()->getFullName();
            // "tte" for Signals and Gateways
            if (strstr(name, "tte"))
            {
                fprintf(feventlog, "S %s TC %s ID %ld N %s P %s T %s SM \n", msg->getFullName(), msg->getClassName(),
                        pkt->getEncapsulationTreeId(),
                        mod->getParentModule()->getParentModule()->getParentModule()->getFullName(),
                        mod->getParentModule()->getFullName(), SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));
            }
            else
            {
                fprintf(feventlog, "S %s TC %s ID %ld N %s P %s T %s SM \n", msg->getFullName(), msg->getClassName(),
                        pkt->getEncapsulationTreeId(), mod->getParentModule()->getParentModule()->getFullName(),
                        mod->getParentModule()->getFullName(), SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));
            }
        }
        //Message arrives at Gateway
        if (mod && strstr(mod->getFullName(), "gatewayApp"))
        {
            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);
            fprintf(feventlog, "S %s TC %s ID %ld N %s T %s SM startMsg\n", msg->getFullName(), msg->getClassName(),
                    pkt->getEncapsulationTreeId(), mod->getParentModule()->getParentModule()->getFullName(),
                    SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));

        }
        //Message is created
        else if (mod
                && (strstr(mod->getFullName(), "app") || strstr(mod->getFullName(), "App")
                        || strstr(mod->getFullName(), "APP")))
        {
            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);
            fprintf(feventlog, "S %s TC %s ID %ld N %s T %s SM startMsg\n", msg->getFullName(), msg->getClassName(),
                    pkt->getEncapsulationTreeId(),
                    mod->getParentModule() ? mod->getParentModule()->getFullName() : mod->getFullName(),
                    SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));

        }
    }

}

void GCTAEventlogManager::connectionCreated(omnetpp::cGate *srcgate)
{
    if (recordEventlog && feventlog)
    {
        omnetpp::cGate *destgate = srcgate->getNextGate();
        //CoRE4INET Connections
        if (strstr(srcgate->getOwnerModule()->getFullName(), "node")
                && strstr(destgate->getOwnerModule()->getFullName(), "switch"))
        {
            fprintf(feventlog, "TOPO SG %s DG %s P%s\n", srcgate->getOwnerModule()->getFullName(),
                    destgate->getOwnerModule()->getFullName(), destgate->getFullName());
        }
        //Signals and Gateway Connections
        if (strstr(srcgate->getOwnerModule()->getFullName(), "gateway")
                && strstr(destgate->getOwnerModule()->getFullName(), "switch"))
        {
            fprintf(feventlog, "TOPO SG %s DG %s P%s\n", srcgate->getOwnerModule()->getFullName(),
                    destgate->getOwnerModule()->getFullName(), destgate->getFullName());
        }
        if (strstr(destgate->getOwnerModule()->getFullName(), "switch"))
        {
            fprintf(feventlog, "TOPO SG %s DG %s P%s\n", srcgate->getOwnerModule()->getFullName(),
                    destgate->getOwnerModule()->getFullName(), destgate->getFullName());
        }
    }
}
