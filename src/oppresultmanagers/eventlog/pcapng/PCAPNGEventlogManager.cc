#include "PCAPNGEventlogManager.h"

#include "pcapng.h"

#include "inet/common/serializer/SerializerBase.h"

Register_Class(PCAPNGEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_FILE, "pcapng-file", CFG_FILENAME,
        "${resultdir}/${configname}-${runnumber}.pcapng", "Name of the PCAPNG file to generate.");

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_INTERFACES, "pcapng-interfaces", CFG_STRING, "\"\"",
        "List of modules (comma or space separated) that appear as an interface in the pcapng file");

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_CAPTURELENGTH, "pcapng-capturelength", CFG_INT, "10000",
        "Maximum length of packet that is being captured (length will be still correct)");

PCAPNGEventlogManager::PCAPNGEventlogManager()
{
    recordEventlog = false;
    buffer = malloc(60000);
    pcapwriter = new PCAPNGWriter(buffer, 60000);

    recordEventlog = omnetpp::cConfiguration::parseBool(
            omnetpp::getEnvir()->getConfig()->getConfigEntry("record-eventlog").getValue(), "false");
    filename = omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_EVENTLOG_PCAPNG_FILE).c_str();
    pcapwriter->openFile(filename.c_str());

    recordingStarted = false;
    capture_length =
            static_cast<size_t>(omnetpp::getEnvir()->getConfig()->getAsInt(CFGID_EVENTLOG_PCAPNG_CAPTURELENGTH));
}

PCAPNGEventlogManager::~PCAPNGEventlogManager()
{
    if (recordingStarted)
    {
        stopRecording();
    }
    pcapwriter->closeFile();
}

void PCAPNGEventlogManager::startRecording()
{
    pcapwriter->openSection("Simulation (no real Hardware)", "", "OMNeT++");

    std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_EVENTLOG_PCAPNG_INTERFACES);
    std::vector<std::string> interfaceModules = omnetpp::cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator interfaceModule = interfaceModules.begin();
            interfaceModule != interfaceModules.end(); ++interfaceModule)
    {
        omnetpp::cModule *module = omnetpp::getSimulation()->getModuleByPath((*interfaceModule).c_str());
        if (!module)
        {
            throw omnetpp::cRuntimeError("error in ini file (pcapng-interfaces option): Module \"%s\" cannot be found",
                    (*interfaceModule).c_str());
        }
        interfaceMap[module] = pcapwriter->addInterface("", module->getFullPath(), capture_length,
                static_cast<uint8_t>(abs(omnetpp::simTime().getScaleExp())));
    }
    recordingStarted = true;
}

void PCAPNGEventlogManager::stopRecording()
{
    if (recordingStarted)
    {
        pcapwriter->closeSection();
        recordingStarted = false;
    }
}

void PCAPNGEventlogManager::flush()
{

}

void PCAPNGEventlogManager::simulationEvent(__attribute__((__unused__))                   omnetpp::cEvent *event)
{

}

void PCAPNGEventlogManager::beginSend(omnetpp::cMessage *msg)
{
    if (recordEventlog)
    {
        if (!recordingStarted)
        {
            startRecording();
        }

        if (omnetpp::cPacket* pkt = dynamic_cast<omnetpp::cPacket*>(msg))
        {
            if (dynamic_cast<omnetpp::cDatarateChannel*>(msg->getSenderGate()->findTransmissionChannel()))
            {
                if (interfaceMap.find(msg->getSenderModule()) != interfaceMap.end())
                {
                    char buffer[10000];
                    inet::serializer::Buffer wb(buffer, sizeof(buffer));
                    inet::serializer::Context c;
                    c.throwOnSerializerNotFound = false;
                    inet::serializer::SerializerBase::lookupAndSerialize(pkt, wb, c, inet::serializer::LINKTYPE,
                            inet::serializer::LINKTYPE_ETHERNET, capture_length);
                    EV << wb.getPos() << std::endl;
                    pcapwriter->addEnhancedPacket(static_cast<uint32_t>(interfaceMap[msg->getSenderModule()]), true,
                            static_cast<uint64_t>(msg->getSendingTime().raw()), pkt->getByteLength(), wb.getPos(),
                            buffer);
                }
            }
        }
    }
}
