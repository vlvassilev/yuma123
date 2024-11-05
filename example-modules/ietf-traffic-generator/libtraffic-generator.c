#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <arpa/inet.h>
//#include <linux/crc32.h>
#include <zlib.h>
#include "libtraffic-generator.h"
#include "timespec-math.h"
#include "yang-date-and-time.h"

static unsigned char hexchar2byte(char hexchar)
{
    char byte;
    if(hexchar>='0' && hexchar<='9') {
        byte = hexchar - '0';
    } else if (hexchar>='A' && hexchar<='F') {
        byte = hexchar - 'A' + 10;
    } else if (hexchar>='a' && hexchar<='f') {
        byte = hexchar - 'a' + 10;
    } else {
        assert(0);
    }
    return byte;
}

static char byte2hexchar(unsigned char byte)
{
    char hexchar;
    if(byte>=0 && byte<=9) {
        hexchar = byte + '0';
    } else if (byte>=0xA && byte<=0xF) {
        hexchar = byte + 'A'-0xA;
    } else {
        assert(0);
    }
    return hexchar;
}

static void hexstr2bin(char* hexstr, uint8_t* data)
{
    unsigned int i;
    unsigned int len;

    len = strlen(hexstr)/2;

    for(i=0;i<len;i++) {
        data[i] = (hexchar2byte(hexstr[i*2])<<4) | (hexchar2byte(hexstr[i*2+1]));
    }
}

static void bin2hexstr(uint8_t* data, uint32_t len, char* hexstr)
{
    unsigned int i;


    for(i=0;i<len;i++) {
        hexstr[2*i] = byte2hexchar(data[i]>>4);
        hexstr[2*i+1] = byte2hexchar(data[i]&0xF);
    }
    hexstr[2*len]=0;
}

/* Frame definitions */

#define DST_MAC_OFFSET 0
#define SRC_MAC_OFFSET 6
#define IPV4_PDU_OFFSET 14
#define IPV4_LEN_OFFSET 16
#define IPV4_HEADER_LEN 20
#define IPV4_TTL_OFFSET 22
#define IPV4_HEADER_CHECKSUM_OFFSET 24
#define SRC_IPV4_ADDRESS_OFFSET 26
#define DST_IPV4_ADDRESS_OFFSET 30
#define SRC_IPV4_UDP_PORT_OFFSET 34
#define DST_IPV4_UDP_PORT_OFFSET 36
#define IPV4_UDP_LEN_OFFSET 38
#define IPV4_UDP_CHECKSUM_OFFSET 40
#define IPV4_UDP_PAYLOAD_OFFSET 42

static void update_dst_mac_address(uint8_t* frame_data, uint32_t frame_size, char* mac_address)
{
    unsigned int mac[6];
    uint8_t* ptr;
    int i;
    int ret;

    ret = sscanf(mac_address, "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    assert(ret==6);
    ptr = frame_data+DST_MAC_OFFSET;
    for(i=0;i<6;i++) {
        ptr[i] = mac[i];
    }
}

static void update_src_mac_address(uint8_t* frame_data, uint32_t frame_size, char* mac_address)
{
    unsigned int mac[6];
    uint8_t* ptr;
    int i;
    int ret;

    ret = sscanf(mac_address, "%x:%x:%x:%x:%x:%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    assert(ret==6);
    ptr = frame_data+SRC_MAC_OFFSET;
    for(i=0;i<6;i++) {
        ptr[i] = mac[i];
    }
}

static void update_src_ipv4_address(uint8_t* frame_data, uint32_t frame_size, char* ipv4_address)
{

    uint8_t* ptr;
    struct in_addr addr = {};

    if(!inet_aton(ipv4_address, &addr)) {
        fprintf(stderr, "Invalid IP address: %s\n", ipv4_address);
        assert(0);
    }
    ptr = frame_data+SRC_IPV4_ADDRESS_OFFSET;
    memcpy(ptr,&addr.s_addr,4);
}

static void update_dst_ipv4_address(uint8_t* frame_data, uint32_t frame_size, char* ipv4_address)
{

    uint8_t* ptr;
    struct in_addr addr = {};

    if(!inet_aton(ipv4_address, &addr)) {
        fprintf(stderr, "Invalid IP address: %s\n", ipv4_address);
        assert(0);
    }
    ptr = frame_data+DST_IPV4_ADDRESS_OFFSET;
    memcpy(ptr,&addr.s_addr,4);
}

static void update_dst_ipv4_udp_port(uint8_t* frame_data, uint32_t frame_size, char* ipv4_udp_port)
{
    uint8_t* ptr;
    uint16_t port;
    port = atoi(ipv4_udp_port);
    ptr = frame_data+DST_IPV4_UDP_PORT_OFFSET;
    ptr[0] = port >>8;
    ptr[1] = port & 0xFF;
}

static void update_src_ipv4_udp_port(uint8_t* frame_data, uint32_t frame_size, char* ipv4_udp_port)
{
    uint8_t* ptr;
    uint16_t port;
    port = atoi(ipv4_udp_port);
    ptr = frame_data+SRC_IPV4_UDP_PORT_OFFSET;
    ptr[0] = port >>8;
    ptr[1] = port & 0xFF;
}

static void update_ipv4_ttl(uint8_t* frame_data, uint32_t frame_size, char* ipv4_ttl)
{
    uint8_t* ptr;
    ptr = frame_data+IPV4_TTL_OFFSET;
    ptr[0] = atoi(ipv4_ttl);
}

static void update_ipv4_len(uint8_t* frame_data, uint32_t frame_size)
{
    uint8_t* ptr;
    uint32_t len;
    len = frame_size - 14 - 4;
    ptr = frame_data+IPV4_LEN_OFFSET;
    ptr[0] = len >> 8;
    ptr[1] = len & 0xFF;
}

static void update_ipv4_header_checksum(uint8_t* frame_data, uint32_t frame_size)
{
    unsigned int i;
    uint32_t w;
    uint8_t* ptr;
    uint32_t sum;

    ptr = frame_data+IPV4_HEADER_CHECKSUM_OFFSET;
    ptr[0] = 0;
    ptr[1] = 0;

    sum = 0;
    for(i=0;i<(IPV4_HEADER_LEN/2);i++) {
        w = (((uint32_t)frame_data[IPV4_PDU_OFFSET+i*2])<<8) + frame_data[IPV4_PDU_OFFSET+i*2+1];
        sum += w;
        if(sum>0xFFFF) {
            sum = sum - 0xFFFF;
        }
    }
    ptr[0] = ((~sum) >> 8) & 0xFF;
    ptr[1] = (~sum) & 0xFF;
}
static void update_ipv4_udp_len(uint8_t* frame_data, uint32_t frame_size)
{
    uint8_t* ptr;
    uint32_t len;
    len = frame_size - 14 - 4 - IPV4_HEADER_LEN;
    ptr = frame_data+IPV4_UDP_LEN_OFFSET;
    ptr[0] = len >> 8;
    ptr[1] = len & 0xFF;
}

static void update_ipv4_udp_checksum(uint8_t* frame_data, uint32_t frame_size)
{
    /* Checksum of 0 is what rfc2544 specifies */
    uint8_t* ptr;
    ptr = frame_data+IPV4_UDP_CHECKSUM_OFFSET;
    ptr[0] = 0;
    ptr[1] = 0;
}

static void update_crc(uint8_t* frame_data, uint32_t frame_size)
{
    uint32_t crc_final;
    uint32_t crc_initial=0;
// linux API:   crc_final = crc32(crc_initial ^ 0xffffffff, frame_data, frame_size-4) ^ 0xffffffff;
    crc_final = crc32(crc_initial, frame_data, frame_size-4);
    frame_data[frame_size-4]=(crc_final>>0)&0xFF;
    frame_data[frame_size-3]=(crc_final>>8)&0xFF;
    frame_data[frame_size-2]=(crc_final>>16)&0xFF;
    frame_data[frame_size-1]=(crc_final>>24)&0xFF;
}

/* valid 64 byte rfc2544 sec C.2.6.4 testframe */
static char* rfc2544_testframe =
/* DATAGRAM HEADER*/
"123456789ABC" /* locally administered DST MAC address */
"DEF012345678" /* locally administered SRC MAC address */
"0800" /* type */
/* IP HEADER */
"4500002E000000000A11BC01C0000201C0000202"
/* UDP HEADER */
"C0200007001A0000"
/* UDP DATA */
"000102030405060708090A0B0C0D0E0F10110DBD8EC6";

char* traffic_generator_make_testframe(uint32_t frame_size, char* frame_data_hexstr, char* src_mac_address, char* dst_mac_address, char* src_ipv4_address, char* dst_ipv4_address, char* ipv4_ttl, char* src_ipv4_udp_port, char* dst_ipv4_udp_port)
{
    unsigned int i;
    uint8_t* frame_data;
    uint8_t* ptr;
    char* result_frame_hexstr;

    frame_data = malloc(frame_size);
    memset(frame_data,0,frame_size);

    hexstr2bin(rfc2544_testframe, frame_data);

    /* payload - incrementing octets */
    for(i=0;i<(frame_size-IPV4_UDP_PAYLOAD_OFFSET-4);i++) {
        frame_data[IPV4_UDP_PAYLOAD_OFFSET+i]=i&0xFF;
    }

    if(frame_data_hexstr) {
        assert(strlen(frame_data_hexstr)<=(frame_size*2));
        hexstr2bin(frame_data_hexstr,frame_data);
    }

    if(src_ipv4_udp_port) {
        update_src_ipv4_udp_port(frame_data, frame_size, src_ipv4_udp_port);
    }

    if(dst_ipv4_udp_port) {
        update_dst_ipv4_udp_port(frame_data, frame_size, dst_ipv4_udp_port);
    }

    update_ipv4_udp_len(frame_data, frame_size);
    update_ipv4_udp_checksum(frame_data, frame_size);

    if(src_ipv4_address) {
        update_src_ipv4_address(frame_data, frame_size, src_ipv4_address);
    }

    if(dst_ipv4_address) {
        update_dst_ipv4_address(frame_data, frame_size, dst_ipv4_address);
    }

    if(ipv4_ttl) {
        update_ipv4_ttl(frame_data, frame_size, ipv4_ttl);
    }
    update_ipv4_len(frame_data, frame_size);
    update_ipv4_header_checksum(frame_data, frame_size);

    if(src_mac_address) {
        update_src_mac_address(frame_data, frame_size, src_mac_address);
    }

    if(dst_mac_address) {
        update_dst_mac_address(frame_data, frame_size, dst_mac_address);
    }
    update_crc(frame_data, frame_size);

    result_frame_hexstr = malloc(frame_size*2+1);
    bin2hexstr(frame_data,frame_size,result_frame_hexstr);
    free(frame_data);

    return result_frame_hexstr;
}

traffic_generator_t* traffic_generator_init(uint64_t interface_speed, char* realtime_epoch, uint32_t frame_size, char* frame_data_hexstr, uint32_t interframe_gap, uint32_t interburst_gap, uint32_t frames_per_burst, uint32_t bursts_per_stream, uint64_t total_frames, char* testframe_type)
{
    unsigned int i;
    traffic_generator_t* tg;
    int ret;

    tg = (traffic_generator_t*)malloc(sizeof(traffic_generator_t));
    memset(tg,0,sizeof(traffic_generator_t));

    tg->streams_num = 1;
    tg->streams = malloc(sizeof(stream_t)*tg->streams_num);
    memset(tg->streams,0,sizeof(stream_t)*tg->streams_num);

    tg->nsec_per_octet = ((double)8*1000000000) / interface_speed; /* e.g. 8.0 for 1Gb */
    tg->octets_per_sec = interface_speed / 8;
    tg->total_frames = total_frames; /* hardcode to 10 sec at 50% rate with minimum packets 10*(1000000000/((20+64)*8))/2=7440476 */
    tg->total_frame_index = 0;

    for(i=0;i<tg->streams_num;i++) {
        tg->streams[i].bursts_per_stream = bursts_per_stream;
        tg->streams[i].frame_size = frame_size;
        tg->streams[i].frame_data = malloc(tg->streams[i].frame_size);
        memset(tg->streams[i].frame_data,0,frame_size);
        assert(strlen(frame_data_hexstr)/2 <= frame_size);
        hexstr2bin(frame_data_hexstr,tg->streams[i].frame_data);

        tg->streams[i].interframe_gap=interframe_gap;
        tg->streams[i].frames_per_burst=frames_per_burst;
        tg->streams[i].interburst_gap=interburst_gap;
        tg->streams[i].frames_per_burst=frames_per_burst;
        if(interburst_gap!=0) {
            tg->streams[i].interstream_gap=interburst_gap;
        } else {
            tg->streams[i].interstream_gap=interframe_gap;
        }
        if(testframe_type!=NULL) {
            tg->streams[i].testframe_type=1;
            if(0==strcmp(testframe_type,"dynamic")) {
                tg->streams[i].testframe_type_dynamic=1;
            }
        }
    }

    ret = yang_date_and_time_to_ieee_1588(realtime_epoch, &tg->sec, &tg->nsec);
    assert(ret==0);

    return tg;
}

static void frame_time_stamp(traffic_generator_t* tg)
{
    uint8_t* timestamp;
    unsigned int offset;
    offset = tg->streams[tg->stream_index].frame_size - 10 - 4 /* crc */;
    timestamp = (uint8_t*)&tg->streams[tg->stream_index].frame_data[offset];
    timestamp[0] = (tg->sec&0x0000FF0000000000)>>40;
    timestamp[1] = (tg->sec&0x000000FF00000000)>>32;
    timestamp[2] = (tg->sec&0x00000000FF000000)>>24;
    timestamp[3] = (tg->sec&0x0000000000FF0000)>>16;
    timestamp[4] = (tg->sec&0x000000000000FF00)>>8;
    timestamp[5] = (tg->sec&0x00000000000000FF)>>0;

    timestamp[6] = (tg->nsec&0x00000000FF000000)>>24;
    timestamp[7] = (tg->nsec&0x0000000000FF0000)>>16;
    timestamp[8] = (tg->nsec&0x000000000000FF00)>>8;
    timestamp[9] = (tg->nsec&0x00000000000000FF)>>0;
}

static void frame_sequence_stamp(traffic_generator_t* tg)
{
    uint64_t index;
    uint8_t* seqnum;
    unsigned int offset;
    offset = tg->streams[tg->stream_index].frame_size - 18 - 4 /* crc */;
    seqnum = (uint8_t*)&tg->streams[tg->stream_index].frame_data[offset];
    index = tg->total_frame_index-1;
    seqnum[0] = (index&0xFF00000000000000)>>56;
    seqnum[1] = (index&0x00FF000000000000)>>48;
    seqnum[2] = (index&0x0000FF0000000000)>>40;
    seqnum[3] = (index&0x000000FF00000000)>>32;
    seqnum[4] = (index&0x00000000FF000000)>>24;
    seqnum[5] = (index&0x0000000000FF0000)>>16;
    seqnum[6] = (index&0x000000000000FF00)>>8;
    seqnum[7] = (index&0x00000000000000FF)>>0;
}

static void frame_udp_checksum_update(traffic_generator_t* tg)
{
    uint8_t* checksum;
    unsigned int offset;
    offset = 14/*ethernet*/+20+6; /* no vlan */
    checksum = (uint8_t*)&tg->streams[tg->stream_index].frame_data[offset];
    checksum[0] = 0;
    checksum[1] = 0;
    /* TODO - zero is OK */
}

int traffic_generator_get_frame(traffic_generator_t* tg, uint32_t* frame_size, uint8_t** frame_data, uint64_t* tx_time_sec, uint32_t* tx_time_nsec)
{
    struct timespec start, delta, next;
    double delta_nsec;
    uint64_t delta_time_octets;

    if((tg->total_frames>0) && (tg->total_frame_index==tg->total_frames)) {
        return 1;
    } else {
        tg->total_frame_index++;
    }

    *frame_data = tg->streams[tg->stream_index].frame_data;
    *frame_size = tg->streams[tg->stream_index].frame_size;
    *tx_time_sec = tg->sec;
    *tx_time_nsec = tg->nsec;

    start.tv_sec = tg->sec;
    start.tv_nsec = tg->nsec;

    /* update time and indexes for next transmission */
    if((tg->frame_index+1) < tg->streams[tg->stream_index].frames_per_burst) {
        tg->frame_index++;
        delta_time_octets=(*frame_size+tg->streams[tg->stream_index].interframe_gap);
    } else if((tg->burst_index+1) < tg->streams[tg->stream_index].bursts_per_stream) {
        tg->frame_index=0;
        tg->burst_index++;
        delta_time_octets=(*frame_size+tg->streams[tg->stream_index].interburst_gap);
    } else {
        delta_time_octets=(*frame_size+tg->streams[tg->stream_index].interstream_gap);
        tg->frame_index=0;
        tg->burst_index=0;
        tg->stream_index=(tg->stream_index+1)%tg->streams_num;
    }

    delta.tv_sec=(delta_time_octets/tg->octets_per_sec);
    delta_nsec = (delta_time_octets-delta.tv_sec*tg->octets_per_sec)*(double)tg->nsec_per_octet + tg->nsec_fraction;
    delta.tv_sec += (long)delta_nsec/1000000000;
    delta.tv_nsec=(long)delta_nsec%1000000000;
    tg->nsec_fraction = delta_nsec - (long)delta_nsec;

    timespec_add(&start, &delta, &next);

    if(tg->streams[tg->stream_index].testframe_type && tg->streams[tg->stream_index].testframe_type_dynamic) {

        frame_time_stamp(tg);

        frame_sequence_stamp(tg);

        frame_udp_checksum_update(tg);

        update_crc(tg->streams[tg->stream_index].frame_data, tg->streams[tg->stream_index].frame_size);

    }

    tg->sec = next.tv_sec;
    tg->nsec = next.tv_nsec;
    return 0;
}

void traffic_generator_set_epoch(traffic_generator_t* tg, uint64_t sec, uint32_t nsec)
{
    tg->sec=sec;
    tg->nsec=nsec;
}

