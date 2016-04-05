/*
 * pcapng.h
 *
 *  Created on: 05.04.2016
 *      Author: tillsteinbach
 */

#ifndef PCAPNG_H_
#define PCAPNG_H_

/*
 * Block types.
 */

/*
 * Common part at the beginning of all blocks.
 */
struct block_header {
    uint32_t block_type;
    uint32_t total_length;
};

/*
 * Common trailer at the end of all blocks.
 */
struct block_trailer {
    uint32_t total_length;
};

/*
 * Common options.
 */
#define OPT_ENDOFOPT    0   /* end of options */
#define OPT_COMMENT 1   /* comment string */

/*
 * Option header.
 */
struct option_header {
    u_short     option_code;
    u_short     option_length;
};


/*
 * Structures for the part of each block type following the common
 * part.
 */

/*
 * Section Header Block.
 */
#define BT_SHB          0x0A0D0D0A

struct section_header_block {
    uint32_t    byte_order_magic;
    u_short     major_version;
    u_short     minor_version;
    u_int64_t   section_length;
    /* followed by options and trailer */
};

/*
 * Options in the SHB.
 */
#define SEC_HARDWARE 2
#define SEC_OS 3
#define SEC_USERAPPL 4

/*
 * Byte-order magic value.
 */
#define BYTE_ORDER_MAGIC    0x1A2B3C4D

/*
 * Current version number.  If major_version isn't PCAP_NG_VERSION_MAJOR,
 * that means that this code can't read the file.
 */
#define PCAP_NG_VERSION_MAJOR   1

#define PCAP_NG_VERSION_MINOR   0

/*
 * Interface Description Block.
 */
#define BT_IDB          0x00000001

struct interface_description_block {
    u_short     linktype;
    u_short     reserved;
    uint32_t    snaplen;
    /* followed by options and trailer */
};

/*
 * Options in the IDB.
 */
#define IF_NAME     2   /* interface name string */
#define IF_DESCRIPTION  3   /* interface description string */
#define IF_IPV4ADDR 4   /* interface's IPv4 address and netmask */
#define IF_IPV6ADDR 5   /* interface's IPv6 address and prefix length */
#define IF_MACADDR  6   /* interface's MAC address */
#define IF_EUIADDR  7   /* interface's EUI address */
#define IF_SPEED    8   /* interface's speed, in bits/s */
#define IF_TSRESOL  9   /* interface's time stamp resolution */
#define IF_TZONE    10  /* interface's time zone */
#define IF_FILTER   11  /* filter used when capturing on interface */
#define IF_OS       12  /* string OS on which capture on this interface was done */
#define IF_FCSLEN   13  /* FCS length for this interface */
#define IF_TSOFFSET 14  /* time stamp offset for this interface */

/*
 * Linktypes in the IDB.
 */
#define LINKTYPE_ETHERNET   1

/*
 * Enhanced Packet Block.
 */
#define BT_EPB          0x00000006

struct enhanced_packet_block {
    uint32_t interface_id;
    uint32_t timestamp_high;
    uint32_t timestamp_low;
    uint32_t caplen;
    uint32_t len;
    /* followed by packet data, options, and trailer */
};


#endif /* PCAPNG_H_ */
