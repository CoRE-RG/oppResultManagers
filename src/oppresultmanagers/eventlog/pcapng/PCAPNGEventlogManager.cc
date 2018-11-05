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

#include "oppresultmanagers/eventlog/pcapng/PCAPNGEventlogManager.h"

#include "oppresultmanagers/eventlog/pcapng/pcapng.h"
#include "oppresultmanagers/utilities/fileutil.h"

#include "inet/common/serializer/SerializerBase.h"

#ifdef WITH_ETHERNET
#include "inet/linklayer/ethernet/EtherFrame.h"
#endif

#ifdef WITH_CAN_COMMON
#include "fico4omnet/linklayer/can/messages/CanDataFrame_m.h"
#endif

Register_Class(PCAPNGEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_FILE, "pcapng-file", CFG_FILENAME,
        "${resultdir}/${configname}-${runnumber}.pcapng", "Name of the PCAPNG file to generate.");

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_INTERFACES, "pcapng-interfaces", CFG_STRING, "\"\"",
        "List of gates (comma or space separated) that appear as an interface in the pcapng file, you can group gates together in one interface using = (e.g. net1.module1.mac.phys$i=eth0, net1.module1.mac.phys$o=eth0, net1.module2.mac.phys$i=eth1");

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_CAPTURELENGTH, "pcapng-capturelength", CFG_INT, "10000",
        "Maximum length of packet that is being captured (length will be still correct)");

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_ETHERNETRAW, "pcapng-ethernet-netanalyzer", CFG_BOOL, "false",
        "Record Ethernet as raw frames (with preamble and SFD) in netanalyzer format");

PCAPNGEventlogManager::PCAPNGEventlogManager()
{
    envir = omnetpp::getEnvir();
    envir->addLifecycleListener(this);
}

PCAPNGEventlogManager::~PCAPNGEventlogManager()
{
    if (recordingStarted)
    {
        stopRecording();
    }
    pcapwriter->closeFile();

    for (std::map<std::string, Interface*>::iterator interface = interfaces.begin(); interface != interfaces.end();
            ++interface)
    {
        delete (*interface).second;
    }
    interfaces.clear();
    interfaceMap.clear();
}

void PCAPNGEventlogManager::configure()
{
    recordEventlog = false;
    buffer = malloc(60000);
    pcapwriter = new PCAPNGWriter(buffer, 60000);

    recordEventlog = omnetpp::cConfiguration::parseBool(
            envir->getConfig()->getConfigEntry("record-eventlog").getValue(), "false");
    filename = envir->getConfig()->getAsFilename(CFGID_EVENTLOG_PCAPNG_FILE).c_str();
    ethernetRaw = envir->getConfig()->getAsBool(CFGID_EVENTLOG_PCAPNG_ETHERNETRAW, false);

    removeFile(filename.c_str(), "old pcapng file");
    mkPath(directoryOf(filename.c_str()).c_str());

    pcapwriter->openFile(filename.c_str());

    recordingStarted = false;
    capture_length =
            static_cast<size_t>(envir->getConfig()->getAsInt(CFGID_EVENTLOG_PCAPNG_CAPTURELENGTH));
}

void PCAPNGEventlogManager::lifecycleEvent(SimulationLifecycleEventType eventType, cObject *details)
{
    switch (eventType) {
        case omnetpp::LF_PRE_NETWORK_SETUP:
            configure();
            break;
    }
}

#if OMNETPP_VERSION >= 0x0501
const char *PCAPNGEventlogManager::getFileName() const
{
    return filename.c_str();
}
#endif

void PCAPNGEventlogManager::startRecording()
{
    pcapwriter->openSection("Simulation (no real Hardware)", "", "OMNeT++");

    std::string cfgobj = omnetpp::getEnvir()->getConfig()->getAsString(CFGID_EVENTLOG_PCAPNG_INTERFACES);
    std::vector<std::string> interfaceModules = omnetpp::cStringTokenizer(cfgobj.c_str(), ", ").asVector();
    for (std::vector<std::string>::const_iterator interfaceModule = interfaceModules.begin();
            interfaceModule != interfaceModules.end(); ++interfaceModule)
    {
        std::size_t pos = (*interfaceModule).rfind('=');
        std::string gatePath;
        std::string interfaceName;
        if (pos != std::string::npos)
        {
            gatePath = (*interfaceModule).substr(0, pos);
            interfaceName = (*interfaceModule).substr(pos + 1);
        }
        else
        {
            gatePath = (*interfaceModule);
        }

        pos = gatePath.rfind('.');
        std::string modulePath;
        std::string gateName;
        if (pos != std::string::npos)
        {
            modulePath = gatePath.substr(0, pos);
            gateName = gatePath.substr(pos + 1);
        }
        else
        {
            throw omnetpp::cRuntimeError("PCAPEventlogManager:  \"%s\" is no valid gate definition", gatePath.c_str());
        }
        //TODO: allow kind of autodetection (e.g. find mac modules in nodes, have to find a clever way to do that)
        omnetpp::cModule *module = omnetpp::getSimulation()->getModuleByPath(modulePath.c_str());
        if (!module)
        {
            throw omnetpp::cRuntimeError(
                    "PCAPEventlogManager: error in ini file (pcapng-interfaces option): Module \"%s\" cannot be found",
                    modulePath.c_str());
        }
        if (!module->isSimple())
        {
            throw omnetpp::cRuntimeError(
                    "PCAPEventlogManager: Sorry, module \"%s\" is no simple module. We can only capture packets at simple modules",
                    modulePath.c_str());
        }
        omnetpp::cGate *gate = module->gate(gateName.c_str());
        if (!gate)
        {
            throw omnetpp::cRuntimeError(
                    "PCAPEventlogManager: error in ini file (pcapng-interfaces option): Gate \"%s\" of module \"%s\" cannot be found",
                    gateName.c_str(), modulePath.c_str());
        }
        uint64_t speed = 0;
        omnetpp::cChannel* channel = nullptr;
        if (gate->getType() == cGate::INPUT)
        {
            channel = gate->findIncomingTransmissionChannel();
        }
        else
        {
            channel = gate->findTransmissionChannel();
        }
        if (omnetpp::cDatarateChannel* datarateChannel = dynamic_cast<omnetpp::cDatarateChannel*>(channel))
        {
            speed = static_cast<uint64_t>(datarateChannel->getDatarate());
        }
        Interface* newInterface = nullptr;
        if (interfaceName.length() > 0)
        {
            std::map<std::string, Interface*>::iterator interface = interfaces.find(interfaceName);
            if (interface != interfaces.end())
            {
                newInterface = (*interface).second;
                newInterface->gates.push_back(gate);
                //If this interface has gates with different speeds, we set the speed to zero
                if (newInterface->speed != speed)
                {
                    newInterface->speed = 0;
                }
            }
        }
        if (!newInterface)
        {
            newInterface = new Interface(interfaceName, speed, IDB_LINKTYPE_NULL);
            newInterface->gates.push_back(gate);
            interfaces.insert(std::pair<std::string, Interface*>(interfaceName, newInterface));
        }
    }
    //now add interfaces
    for (std::map<std::string, Interface*>::iterator interface = interfaces.begin(); interface != interfaces.end();
            ++interface)
    {
        if ((*interface).second->name.length() == 0)
        {
            (*interface).second->name = (*(*interface).second->gates.begin())->getFullPath();
        }

        (*interface).second->id = pcapwriter->addInterface((*interface).second->name, "", (*interface).second->linktype,
                static_cast<uint32_t>(capture_length), static_cast<uint8_t>(abs(omnetpp::simTime().getScaleExp())),
                (*interface).second->speed);
        for (std::list<omnetpp::cGate*>::iterator ifGate = (*interface).second->gates.begin();
                ifGate != (*interface).second->gates.end(); ++ifGate)
        {
            interfaceMap[*ifGate] = (*interface).second;
        }
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

void PCAPNGEventlogManager::simulationEvent(omnetpp::cEvent *event)
{
    if (recordEventlog)
    {
        if (!recordingStarted)
        {
            startRecording();
        }
        //Was it a message?
        if (event->isMessage())
        {
            //Was it a Packet?
            if (omnetpp::cPacket* pkt = dynamic_cast<omnetpp::cPacket*>(event))
            {
                std::map<omnetpp::cGate*, Interface*>::iterator senderGate = interfaceMap.find(pkt->getSenderGate());
                std::map<omnetpp::cGate*, Interface*>::iterator arrivalGate = interfaceMap.find(pkt->getArrivalGate());

                if (inet::EtherPhyFrame* phyFrame = dynamic_cast<inet::EtherPhyFrame*>(pkt))
                {
                    //decapsulate a raw Ethernet frame if not in raw mode
                    if (!ethernetRaw)
                    {
                        pkt = phyFrame->getEncapsulatedPacket();
                    }
                }

                uint16_t linktype = 0;
#ifdef WITH_ETHERNET
                if (dynamic_cast<inet::EtherFrame*>(pkt))
                {
                    linktype = IDB_LINKTYPE_ETHERNET;
                }
                else if (dynamic_cast<inet::EtherPhyFrame*>(pkt))
                {
                    linktype = IDB_LINKTYPE_NETANALYZER_TRANSPARENT;
                }
#endif
#ifdef WITH_CAN_COMMON
                if (dynamic_cast<FiCo4OMNeT::CanDataFrame*>(pkt))
                {
                    linktype = IDB_LINKTYPE_SOCKETCAN;
                }
#endif

                if (linktype)
                {
                    //Serialize if sender or receiver is in capture interfaces
                    if (senderGate != interfaceMap.end() || arrivalGate != interfaceMap.end())
                    {
                        char serializeBuffer[10000];
                        inet::serializer::Buffer wb(serializeBuffer, sizeof(serializeBuffer));
                        inet::serializer::Context c;
                        c.throwOnSerializerNotFound = false;
                        size_t headerOverhead = 0;
                        if (linktype == IDB_LINKTYPE_ETHERNET)
                        {
                            inet::serializer::SerializerBase::lookupAndSerialize(pkt, wb, c, inet::serializer::LINKTYPE,
                                    inet::serializer::LINKTYPE_ETHERNET, static_cast<unsigned int>(capture_length));
                        }
                        else if (linktype == IDB_LINKTYPE_NETANALYZER_TRANSPARENT)
                        {
                            uint32_t header = 0;
                            //Header Version 1
                            header |= (0x1 << 18);
                            wb.writeUint32(header);
                            headerOverhead = sizeof(uint32_t);
                            inet::serializer::SerializerBase::lookupAndSerialize(pkt, wb, c, inet::serializer::PHYTYPE,
                                    inet::serializer::PHYTYPE_ETHERNET, static_cast<unsigned int>(capture_length));
                        }
                        else
                        {
                            inet::serializer::SerializerBase::lookupAndSerialize(pkt, wb, c, inet::serializer::UNKNOWN,
                                    0, static_cast<unsigned int>(capture_length));
                        }

                        //write out if sender is in interfaces
                        if (senderGate != interfaceMap.end())
                        {
                            if (!senderGate->second->isInitialized)
                            {
                                senderGate->second->linktype = linktype;
                                pcapwriter->changeInterfaceDescriptionHeader(senderGate->second->id,
                                        senderGate->second->linktype, static_cast<uint32_t>(capture_length));
                                senderGate->second->isInitialized = true;
                            }
                            else
                            {
                                if (senderGate->second->linktype != linktype)
                                {
                                    throw omnetpp::cRuntimeError(
                                            "Module receives packets with incompatible linktypes, e.g. IP packets and Ethernet packets on the same gate. This is not allowed");
                                }
                            }
                            pcapwriter->addEnhancedPacket(static_cast<uint32_t>(senderGate->second->id), true,
                                    static_cast<uint64_t>(pkt->getSendingTime().raw()),
                                    static_cast<uint32_t>(static_cast<size_t>(pkt->getByteLength()) + headerOverhead), wb.getPos(),
                                    serializeBuffer, pkt->hasBitError());
                        }
                        //write out if receiver is in interfaces
                        if (arrivalGate != interfaceMap.end())
                        {
                            if (!arrivalGate->second->isInitialized)
                            {
                                arrivalGate->second->linktype = linktype;
                                pcapwriter->changeInterfaceDescriptionHeader(arrivalGate->second->id,
                                        arrivalGate->second->linktype, static_cast<uint32_t>(capture_length));
                                arrivalGate->second->isInitialized = true;
                            }
                            else
                            {
                                if (arrivalGate->second->linktype != linktype)
                                {
                                    throw omnetpp::cRuntimeError(
                                            "Module receives packets with incompatible linktypes, e.g. IP packets and Ethernet packets on the same gate. This is not allowed");
                                }
                            }
                            pcapwriter->addEnhancedPacket(static_cast<uint32_t>(arrivalGate->second->id), false,
                                    static_cast<uint64_t>(pkt->getArrivalTime().raw()),
                                    static_cast<uint32_t>(static_cast<size_t>(pkt->getByteLength()) + headerOverhead), wb.getPos(),
                                    serializeBuffer, pkt->hasBitError());
                        }
                    }
                }
            }
        }
    }
}
