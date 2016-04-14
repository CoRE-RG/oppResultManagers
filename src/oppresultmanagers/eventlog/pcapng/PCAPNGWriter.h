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

#include "oppresultmanagers/eventlog/pcapng/pcapng.h"

#include <stddef.h>
#include <stdio.h>
#include <string>

#ifndef PCAPNGWRITER_H_
#define PCAPNGWRITER_H_

enum BlockType
{
    SectionHeader = BT_SHB, InterfaceDescriptionHeader = BT_IDB, EnhancedPacketHeader = BT_EPB,
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
        size_t addInterface(std::string name, std::string description, uint32_t snaplen, uint8_t tsresol,
                uint64_t speed);
        void addEnhancedPacket(uint32_t interfaceId, bool sender, uint64_t timestamp, uint32_t len, uint32_t caplen,
                void* data, bool bitError);

};

#endif /* PCAPNGWRITER_H_ */
