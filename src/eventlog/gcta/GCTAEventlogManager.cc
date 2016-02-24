#include "GCTAEventlogManager.h"

Register_Class(GCTAEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOG_TLOG_FILE, "tlog-file", CFG_FILENAME,
        "${resultdir}/${configname}-${runnumber}.tlog", "Name of the GCTA timing log file to generate.");

GCTAEventlogManager::GCTAEventlogManager()
{
    recordEventlog = false;
    feventlog = nullptr;
}

GCTAEventlogManager::~GCTAEventlogManager()
{
}

void GCTAEventlogManager::configure()
{
    // main switch
    recordEventlog = omnetpp::cConfiguration::parseBool(
            omnetpp::getEnvir()->getConfig()->getConfigEntry("record-eventlog").getValue(), "false");

    // setup filename
    filename = omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_EVENTLOG_TLOG_FILE).c_str();
}

void GCTAEventlogManager::recordSimulation()
{

}

void GCTAEventlogManager::startRun()
{
    if (!feventlog)
    {
        //removeFile(filename.c_str(), "old eventlog file");
        //mkPath(directoryOf(filename.c_str()).c_str());
        FILE *out = fopen(filename.c_str(), "w");
        if (!out)
            throw omnetpp::cRuntimeError("Cannot open eventlog file `%s' for write", filename.c_str());
        ::printf("Recording eventlog to file `%s'...\n", filename.c_str());
        feventlog = out;
    }
}

void GCTAEventlogManager::endRun(__attribute__((__unused__)) bool isError, __attribute__((__unused__)) int resultCode,
        __attribute__((__unused__)) const char *message)
{
    if (feventlog)
    {
        fflush(feventlog);
        fclose(feventlog);

        feventlog = nullptr;
    }
}

bool GCTAEventlogManager::hasRecordingIntervals() const
{
    return false;
}

void GCTAEventlogManager::clearRecordingIntervals()
{

}

void GCTAEventlogManager::flush()
{
    if (recordEventlog)
        fflush(feventlog);
}

void GCTAEventlogManager::simulationEvent(__attribute__((__unused__)) omnetpp::cEvent *event)
{

}

void GCTAEventlogManager::beginSend(omnetpp::cMessage *msg)
{
    if (msg->isPacket() && recordEventlog)
    {

        omnetpp::cModule *mod = msg->getArrivalModule();

        if (mod)
        {
            EV << "Module Name: " << mod->getFullName() << omnetpp::endl;
        }

        //Mac Module Ankunft/Abfahrt
        if (mod && strcmp(mod->getFullName(), "mac") == 0)
        {

            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);

            const char * name = mod->getParentModule()->getParentModule()->getFullName();
            //CoRE4INET
            if (!strstr(name, "tte") || strstr(name, "switch"))
            {
                fprintf(feventlog, "%s TC %s ID %ld N %s p %s t %s et \n", msg->getFullName(), msg->getClassName(),
                        pkt->getEncapsulationTreeId(), mod->getParentModule()->getParentModule()->getFullName(),
                        mod->getParentModule()->getFullName(), SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));
            }
            else
            {
                //Signals and Gateway
                EV << "Name: " << name << omnetpp::endl;
                fprintf(feventlog, "%s TC %s ID %ld N %s p %s t %s et \n", msg->getFullName(), msg->getClassName(),
                        pkt->getEncapsulationTreeId(),
                        mod->getParentModule()->getParentModule()->getParentModule()->getFullName(),
                        mod->getParentModule()->getFullName(), SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));

            }

        }

        //Message is created

        if (mod && strstr(mod->getFullName(), "app"))
        {

            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);
            fprintf(feventlog, "%s TC %s ID %ld N %s t %s et startMsg\n", msg->getFullName(), msg->getClassName(),
                    pkt->getEncapsulationTreeId(),
                    mod->getParentModule() ? mod->getParentModule()->getFullName() : mod->getFullName(),
                    SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));

        }
        //Message arrives at Gateway
        if (mod && strstr(mod->getFullName(), "gatewayApp"))
        {

            //if(hasToRecord){
            omnetpp::cPacket *pkt = static_cast<omnetpp::cPacket *>(msg);
            //EV << "ID: " << pkt->getEncapsulationTreeId() << " Node: " << mod->getFullName() << " MSG: " << msg->getFullName() << " TIME: " << simulation.getSimTime() << endl;
            fprintf(feventlog, "%s TC %s ID %ld N %s t %s et startMsg\n", msg->getFullName(), msg->getClassName(),
                    pkt->getEncapsulationTreeId(), mod->getParentModule()->getParentModule()->getFullName(),
                    SIMTIME_STR(omnetpp::getSimulation()->getSimTime()));

        }

    }

}

void GCTAEventlogManager::connectionCreated(omnetpp::cGate *srcgate)
{
    if (recordEventlog)
    {
        omnetpp::cGate *destgate = srcgate->getNextGate();
        //CoRE4INET Connections
        //if(strstr(srcgate->getOwnerModule()->getFullName(), "node") && strstr(destgate->getOwnerModule()->getFullName(), "switch"))
        //{
        //  fprintf(ftiminglog, "TOPO SG %s DG %s P%s\n",srcgate->getOwnerModule()->getFullName(), destgate->getOwnerModule()->getFullName(), destgate->getFullName() );
        //}
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