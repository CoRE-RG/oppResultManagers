#include "PCAPNGEventlogManager.h"

#include "pcapng.h"

Register_Class(PCAPNGEventlogManager);

Register_PerRunConfigOption(CFGID_EVENTLOG_PCAPNG_FILE, "pcapng-file", CFG_FILENAME,
        "${resultdir}/${configname}-${runnumber}.pcapng", "Name of the PCAPNG file to generate.");

PCAPNGEventlogManager::PCAPNGEventlogManager()
{
    recordEventlog = false;
    pcapfile = nullptr;
}

PCAPNGEventlogManager::~PCAPNGEventlogManager()
{
}

void PCAPNGEventlogManager::startRecording()
{
    if (!pcapfile)
    {
        // main switch
        recordEventlog = omnetpp::cConfiguration::parseBool(
                omnetpp::getEnvir()->getConfig()->getConfigEntry("record-eventlog").getValue(), "false");

        // setup filename
        filename = omnetpp::getEnvir()->getConfig()->getAsFilename(CFGID_EVENTLOG_PCAPNG_FILE).c_str();

        //removeFile(filename.c_str(), "old eventlog file");
        //mkPath(directoryOf(filename.c_str()).c_str());
        pcapfile = fopen(filename.c_str(), "w");
        if (!pcapfile)
            throw omnetpp::cRuntimeError("Cannot open pcapng file `%s' for write", filename.c_str());
        ::printf("Recording pcapng to file `%s'...\n", filename.c_str());

        //Write section header
        struct block_header bh;
        struct section_header_block shb;
        struct block_trailer bt;

        struct option_header oh;
        struct option_header oh_end;
        oh_end.option_code = OPT_ENDOFOPT;
        oh_end.option_length = 0;

        bh.block_type = BT_SHB;

        shb.byte_order_magic = BYTE_ORDER_MAGIC;
        shb.major_version = PCAP_NG_VERSION_MAJOR;
        shb.minor_version = PCAP_NG_VERSION_MINOR;
        shb.section_length = -1;


        std::string appName = "OMNe";
        oh.option_code = SEC_USERAPPL;
        oh.option_length = appName.length();

        bh.total_length = sizeof(bh) + sizeof(shb) + sizeof(oh) + oh.option_length + sizeof(oh_end) + sizeof(bt);
        bt.total_length = bh.total_length;

        fwrite(&bh, sizeof(bh), 1, pcapfile);
        fwrite(&shb, sizeof(shb), 1, pcapfile);

        fwrite(&oh, sizeof(oh), 1, pcapfile);
        fwrite(appName.data(), oh.option_length, 1, pcapfile);
        fwrite(&oh_end, sizeof(oh_end), 1, pcapfile);

        fwrite(&bt, sizeof(bt), 1, pcapfile);

        struct interface_description_block idb;
        bh.block_type = BT_IDB;
        idb.linktype = LINKTYPE_ETHERNET;
        idb.snaplen = 1518;
        bh.total_length = sizeof(bh) + sizeof(idb) + sizeof(bt);
        bt.total_length = bh.total_length;

        fwrite(&bh, sizeof(bh), 1, pcapfile);
        fwrite(&idb, sizeof(idb), 1, pcapfile);
        fwrite(&bt, sizeof(bt), 1, pcapfile);

        struct enhanced_packet_block ehb;

        bh.block_type = BT_EPB;
        ehb.interface_id = 0;
        ehb.timestamp_high = 0; //simtime 64bit
        ehb.timestamp_low = 0; //simtime 64bit
        ehb.caplen = 0;
        ehb.len = 0;

        bh.total_length = sizeof(bh) + sizeof(ehb) + sizeof(bt);
        bt.total_length = bh.total_length;

        fwrite(&bh, sizeof(bh), 1, pcapfile);
        fwrite(&ehb, sizeof(ehb), 1, pcapfile);
        fwrite(&bt, sizeof(bt), 1, pcapfile);
    }
}

void PCAPNGEventlogManager::stopRecording()
{
    if (pcapfile)
    {
        fflush(pcapfile);
        fclose(pcapfile);

        pcapfile = nullptr;
    }
}

void PCAPNGEventlogManager::flush()
{
    if (recordEventlog)
        fflush(pcapfile);
}

void PCAPNGEventlogManager::simulationEvent(__attribute__((__unused__))   omnetpp::cEvent *event)
{

}

void PCAPNGEventlogManager::beginSend(omnetpp::cMessage *msg)
{

}

void PCAPNGEventlogManager::connectionCreated(omnetpp::cGate *srcgate)
{

}
