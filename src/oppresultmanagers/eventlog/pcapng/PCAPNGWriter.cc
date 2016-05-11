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

#include "oppresultmanagers/eventlog/pcapng/PCAPNGWriter.h"

#include <stdexcept>
#include <cstring>

PCAPNGWriter::PCAPNGWriter(void * setBuffer, size_t setBufferSize)
{
    buffer = static_cast<char*>(setBuffer);
    bufferSize = setBufferSize;
    bufferPos = 0;
    blockSize = 0;
    isBlockOpen = false;
    file = nullptr;

    isSectionOpen = false;
    sectionSize = 0;

    hasOptions = false;
    hasEndOption = false;
    numInterfaces = 0;
}

PCAPNGWriter::~PCAPNGWriter()
{
    if (file) {
        closeFile();
    }
}

void PCAPNGWriter::openFile(const char *filename)
{
    if (file) {
        throw std::invalid_argument("file is already open, try closeFile() first.");
    }
    file = fopen(filename, "wb");
}

void PCAPNGWriter::closeFile()
{
    if (!file) {
        throw std::invalid_argument("no file is open, try openFile() first.");
    }
    fclose(file);
    file = nullptr;
}

void PCAPNGWriter::openBlock(BlockType type)
{
    if (isBlockOpen) {
        throw std::invalid_argument("there is already an open block");
    }

    currentBlock = reinterpret_cast<block_header*>(buffer + bufferPos);
    bufferPos += sizeof(block_header);
    currentBlock->block_type = type;

    hasOptions = false;
    isBlockOpen = true;
    hasEndOption = false;
}
void PCAPNGWriter::closeBlock()
{
    if (!isBlockOpen) {
        throw std::invalid_argument("there is no open block");
    }

    block_trailer* trailer = reinterpret_cast<block_trailer*>(buffer + bufferPos);
    bufferPos += sizeof(block_trailer);
    currentBlock->total_length = trailer->total_length = static_cast<uint32_t>(bufferPos);

    //Now everything is ready. We can just write out the whole buffer:
    fwrite(buffer, bufferPos, 1, file);
    sectionSize += bufferPos;
    bufferPos = 0;

    isBlockOpen = false;
}

void PCAPNGWriter::openSection(std::string hardware, std::string os, std::string application)
{
    if (isSectionOpen) {
        throw std::invalid_argument("there is already an open section");
    }

    fgetpos(file, &sectionStart);

    addSectionHeader();
    if (hardware.length() > 0) {
        addPaddedStringOption(SEC_HARDWARE, hardware);
    }
    if (os.length() > 0) {
        addPaddedStringOption(SEC_OS, os);
    }
    if (application.length() > 0) {
        addPaddedStringOption(SEC_USERAPPL, application);
    }
    endOptions();
    closeBlock();

    isSectionOpen = true;
}

void PCAPNGWriter::addSectionHeader()
{

    openBlock(SectionHeader);

    section_header_block* sectionHeader = reinterpret_cast<section_header_block*>(buffer + bufferPos);
    bufferPos += sizeof(section_header_block);
    sectionHeader->byte_order_magic = BYTE_ORDER_MAGIC;
    sectionHeader->major_version = PCAP_NG_VERSION_MAJOR;
    sectionHeader->minor_version = PCAP_NG_VERSION_MINOR;
    sectionHeader->section_length = static_cast<uint64_t>(-1);

}
void PCAPNGWriter::closeSection()
{
    if (!isSectionOpen) {
        throw std::invalid_argument("there is no open section");
    }

    //save current positon
    fpos_t currentPosition;
    fgetpos(file, &currentPosition);
    //go to start of section and set section length
    fsetpos(file, &sectionStart);
    //go to sectionSize
    fseek(file, sizeof(block_header)+sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t), SEEK_CUR);
    //write sectionSize
    fwrite(&sectionSize, sizeof(uint64_t),1,file);
    //go back to previous position
    fsetpos(file, &currentPosition);

    isSectionOpen = false;
    numInterfaces = 0;
}

void PCAPNGWriter::addInterfaceDescriptionHeader(uint16_t linktype, uint32_t snaplen)
{
    openBlock(InterfaceDescriptionHeader);
    interface_description_block* interfaceDescription = reinterpret_cast<interface_description_block*>(buffer
            + bufferPos);
    bufferPos += sizeof(interface_description_block);
    interfaceDescription->linktype = linktype;
    interfaceDescription->snaplen = snaplen;
}

void PCAPNGWriter::changeInterfaceDescriptionHeader(size_t interfaceId, uint16_t linktype, uint32_t snaplen)
{
    //save current positon
    fpos_t currentPosition;
    fgetpos(file, &currentPosition);
    //go to interface
    fsetpos(file, &interfacePos[interfaceId]);
    //Jump over blockheader
    fseek(file, sizeof(block_header), SEEK_CUR);

    //write linktype
    fwrite(&linktype, sizeof(uint16_t),1,file);
    //write snaplen
    fwrite(&snaplen, sizeof(uint32_t),1,file);

    //go back to previous position
    fsetpos(file, &currentPosition);
}

size_t PCAPNGWriter::addInterface(std::string name, std::string description, uint16_t linktype, uint32_t snaplen,
        uint8_t tsresol, uint64_t speed)
{
    fflush(file);
    fpos_t currentPosition;
    fgetpos(file, &currentPosition);
    interfacePos[numInterfaces] = currentPosition;
    addInterfaceDescriptionHeader(linktype, snaplen);
    if (name.length() > 0) {
        addPaddedStringOption(IF_NAME, name);
    }
    if (description.length() > 0) {
        addPaddedStringOption(IF_DESCRIPTION, description);
    }
    addUint8Option(IF_TSRESOL, tsresol);
    if (speed > 0) {
        addUint64Option(IF_SPEED, speed);
    }
    endOptions();
    closeBlock();

    return numInterfaces++;
}

void PCAPNGWriter::addEnhancedPacketHeader(uint32_t interfaceId, uint64_t timestamp, uint32_t caplen, uint32_t len)
{
    openBlock(EnhancedPacketHeader);
    enhanced_packet_block* enhancedPacket = reinterpret_cast<enhanced_packet_block*>(buffer + bufferPos);
    bufferPos += sizeof(enhanced_packet_block);

    enhancedPacket->interface_id = interfaceId;
    enhancedPacket->timestamp_high = static_cast<uint32_t>(timestamp >> 32); //simtime 64bit
    enhancedPacket->timestamp_low = static_cast<uint32_t>(timestamp); //simtime 64bit
    enhancedPacket->caplen = caplen;
    enhancedPacket->len = len;
}

void PCAPNGWriter::addEnhancedPacket(uint32_t interfaceId, bool sender, uint64_t timestamp, uint32_t len,
        uint32_t caplen, void* data, bool bitError)
{
    addEnhancedPacketHeader(interfaceId, timestamp, caplen, len);
    uint32_t flags = 0;
    if (sender) {
        flags |= 0x00000002;
    }
    else {
        flags |= 0x00000001;
    }
    //Biterror
    if (bitError) {
        flags |= 0x01000000;
    }
    std::memcpy((buffer + bufferPos), data, caplen);
    bufferPos += caplen;
    //Now we need to pad to 32bit boundary
    size_t padSize = 4 - (caplen % 4);
    if (padSize < 4) {
        std::memset((buffer + bufferPos), 0, padSize);
        bufferPos += padSize;
    }
    addUint32Option(EP_FLAGS, flags);
    endOptions();
    closeBlock();
}

void PCAPNGWriter::addPaddedStringOption(uint16_t optionCode, std::string string)
{
    hasOptions = true;

    option_header* option = reinterpret_cast<option_header*>(buffer + bufferPos);
    bufferPos += sizeof(option_header);
    option->option_code = optionCode;
    option->option_length = static_cast<uint16_t>(string.length());
    std::memcpy((buffer + bufferPos), string.data(), string.length());
    bufferPos += string.length();
    //Now we need to pad to 32bit boundary
    size_t padSize = 4 - (string.length() % 4);
    if (padSize < 4) {
        std::memset((buffer + bufferPos), 0, padSize);
        bufferPos += padSize;
    }
}

void PCAPNGWriter::addUint64Option(uint16_t optionCode, uint64_t optionValue)
{
    hasOptions = true;

    option_header* option = reinterpret_cast<option_header*>(buffer + bufferPos);
    bufferPos += sizeof(option_header);
    option->option_code = optionCode;
    option->option_length = sizeof(uint64_t);
    uint64_t* value = reinterpret_cast<uint64_t*>(buffer + bufferPos);
    *value = optionValue;
    bufferPos += sizeof(uint64_t);
}

void PCAPNGWriter::addUint32Option(uint16_t optionCode, uint32_t optionValue)
{
    hasOptions = true;

    option_header* option = reinterpret_cast<option_header*>(buffer + bufferPos);
    bufferPos += sizeof(option_header);
    option->option_code = optionCode;
    option->option_length = sizeof(uint32_t);
    uint32_t* value = reinterpret_cast<uint32_t*>(buffer + bufferPos);
    *value = optionValue;
    bufferPos += sizeof(uint32_t);
}

void PCAPNGWriter::addUint8Option(uint16_t optionCode, uint8_t optionValue)
{
    hasOptions = true;

    option_header* option = reinterpret_cast<option_header*>(buffer + bufferPos);
    bufferPos += sizeof(option_header);
    option->option_code = optionCode;
    option->option_length = sizeof(uint8_t);
    uint8_t* value = reinterpret_cast<uint8_t*>(buffer + bufferPos);
    *value = optionValue;
    bufferPos += sizeof(uint8_t);
    //Now we need to pad to 32bit boundary
    size_t padSize = 4 - sizeof(uint8_t);
    std::memset((buffer + bufferPos), 0, padSize);
    bufferPos += padSize;
}

void PCAPNGWriter::endOptions()
{
    if (hasOptions && !hasEndOption) {
        option_header* optionEnd = reinterpret_cast<option_header*>(buffer + bufferPos);
        bufferPos += sizeof(option_header);
        optionEnd->option_code = OPT_ENDOFOPT;
        optionEnd->option_length = 0;
    }
}
