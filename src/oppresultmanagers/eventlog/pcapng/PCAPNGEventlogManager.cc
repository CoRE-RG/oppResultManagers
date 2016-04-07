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
        interfaceMap[module] = pcapwriter->addInterface("", module->getFullPath(),
                static_cast<uint32_t>(capture_length), static_cast<uint8_t>(abs(omnetpp::simTime().getScaleExp())));
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
                //Was the Packet sent over an actual datarate channel
                if (dynamic_cast<omnetpp::cDatarateChannel*>(pkt->getArrivalGate()->findIncomingTransmissionChannel()))
                {
                    std::map<omnetpp::cModule*, size_t>::iterator senderModule = interfaceMap.find(
                            pkt->getSenderModule());
                    std::map<omnetpp::cModule*, size_t>::iterator arrivalModule = interfaceMap.find(
                            pkt->getArrivalModule());
                    //Serialize if sender or receiver is in capture interfaces
                    if (senderModule != interfaceMap.end() || arrivalModule != interfaceMap.end())
                    {
                        char serializeBuffer[10000];
                        inet::serializer::Buffer wb(serializeBuffer, sizeof(serializeBuffer));
                        inet::serializer::Context c;
                        c.throwOnSerializerNotFound = false;
                        inet::serializer::SerializerBase::lookupAndSerialize(pkt, wb, c, inet::serializer::LINKTYPE,
                                inet::serializer::LINKTYPE_ETHERNET, static_cast<unsigned int>(capture_length));

                        //write out if sender is in interfaces
                        if (senderModule != interfaceMap.end())
                        {
                            pcapwriter->addEnhancedPacket(static_cast<uint32_t>(interfaceMap[pkt->getSenderModule()]),
                                    true, static_cast<uint64_t>(pkt->getSendingTime().raw()),
                                    static_cast<uint32_t>(pkt->getByteLength()), wb.getPos(), serializeBuffer);
                        }
                        //write out if receiver is in interfaces
                        if (arrivalModule != interfaceMap.end())
                        {
                            pcapwriter->addEnhancedPacket(static_cast<uint32_t>(interfaceMap[pkt->getArrivalModule()]),
                                    false, static_cast<uint64_t>(pkt->getArrivalTime().raw()),
                                    static_cast<uint32_t>(pkt->getByteLength()), wb.getPos(), serializeBuffer);
                        }
                    }
                }
            }
        }
    }
}
