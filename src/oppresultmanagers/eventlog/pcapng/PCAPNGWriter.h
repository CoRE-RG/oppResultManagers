/*
 * PCAPNGWriter.h
 *
 *  Created on: Apr 6, 2016
 *      Author: steinbach
 */


#include "pcapng.h"

#include <stddef.h>
#include <stdio.h>
#include <string>

#ifndef PCAPNGWRITER_H_
#define PCAPNGWRITER_H_

enum BlockType{
    SectionHeader = BT_SHB,
    InterfaceDescriptionHeader = BT_IDB,
    EnhancedPacketHeader = BT_EPB,
};

class PCAPNGWriter
{
    private:
        char* buffer;
        size_t bufferSize;
        size_t bufferPos;
        FILE *file;

        size_t blockSize;
        bool isBlockOpen;
        block_header *currentBlock;

        bool isSectionOpen;
        size_t sectionSize;
        fpos_t sectionStart;

        bool hasOptions;
        bool hasEndOption;

        size_t numInterfaces;

    public:
        PCAPNGWriter(void * setBuffer, size_t setBufferSize);
        virtual ~PCAPNGWriter();

        void openFile(const char *filename);
        void closeFile();

        void openBlock(BlockType type);
        void closeBlock();

        void addSectionHeader();
        void closeSection();

        void addPaddedStringOption(uint16_t optionCode, std::string string);
        void addUint64Option(uint16_t optionCode, uint64_t optionValue);
        void addUint32Option(uint16_t optionCode, uint32_t optionValue);
        void addUint8Option(uint16_t optionCode, uint8_t optionValue);
        void endOptions();

        void addInterfaceDescriptionHeader(uint32_t snaplen);

        void addEnhancedPacketHeader(uint32_t interfaceId, uint64_t timestamp, uint32_t caplen, uint32_t len);


        void openSection(std::string hardware, std::string os, std::string application);
        size_t addInterface(std::string name, std::string description, uint32_t snaplen, uint8_t tsresol);
        void addEnhancedPacket(uint32_t interfaceId, bool sender, uint64_t timestamp, uint32_t len, uint32_t caplen, void* data);
};

#endif /* PCAPNGWRITER_H_ */
