#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "libtraffic-generator.h"
#include "timespec-math.h"

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

static void hexstr2bin(char* hexstr, uint8_t* data)
{
    unsigned int i;
    unsigned int len;

    len = strlen(hexstr)/2;

    for(i=0;i<len;i++) {
        data[i] = (hexchar2byte(hexstr[i*2])<<4) | (hexchar2byte(hexstr[i*2+1]));
    }
}

int yang_date_and_time_to_1588(char* date_and_time, uint64_t* sec, uint32_t* nsec)
{
    /* YYYY-MM-DDThh:mm:ss.n*Z */
    struct tm tm;
    char nsec_str[]="000000000";

    unsigned int date_and_time_len;
    unsigned int nsec_digits;
    unsigned int i;
    int ret;
    char* ptr;

    *sec=0;
    *nsec=0;

    date_and_time_len = strlen(date_and_time);


    if(date_and_time_len < strlen("YYYY-MM-DDThh:mm:ssZ")) {
        return -1;
    }

    if(date_and_time[date_and_time_len-1] != 'Z') {
        return -1;
    }

#if 0
//#define _XOPEN_SOURCE
    ptr = strptime(date_and_time, "%Y-%m-%dT%H:%M:%S", &tm);
    if(ptr!=date_and_time+strlen("YYYY-MM-DDThh:mm:ss")) {
        return -1;
    }
#else
    memset(&tm, 0, sizeof(struct tm));
    ret = sscanf(date_and_time, "%04d-%02d-%02dT%02d:%02d:%02d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    if(ret!=6) {
        return -1;
    }
    tm.tm_year = tm.tm_year - 1900;
    tm.tm_mon = tm.tm_mon - 1;

    tm.tm_isdst = 0;
    ptr = date_and_time + strlen("YYYY-MM-DDThh:mm:ss");
#endif
    *sec = mktime(&tm);

    if(ptr==(date_and_time+date_and_time_len-1)) {
        return 0;
    }

    if(*ptr!='.') {
        return -1;
    }

    ptr++;

    nsec_digits = date_and_time_len-strlen("YYYY-MM-DDThh:mm:ss.")-strlen("Z");

    for(i=0;i<nsec_digits;i++) {
        if(ptr[i]<'0' || ptr[i]>'9') {
            return -1;
        }
        nsec_str[i] = ptr[i];
    }

    *nsec = atoi(nsec_str);

    return 0;
}


traffic_generator_t* traffic_generator_init(uint64_t interface_speed, char* realtime_epoch, uint32_t frame_size, char* frame_data_hexstr, uint32_t interframe_gap, uint32_t interburst_gap, uint32_t frames_per_burst, uint32_t bursts_per_stream, uint64_t total_frames, char* testframe)
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
        if(testframe!=NULL) {
            tg->streams[i].testframe=1;
            if(0==strcmp(testframe,"testframe-ipv4-udp")) {
                tg->streams[i].testframe_ipv4_udp=1;
            }
        }
    }

    ret = yang_date_and_time_to_1588(realtime_epoch, &tg->sec, &tg->nsec);
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

    if(tg->streams[tg->stream_index].testframe) {

        frame_time_stamp(tg);

        frame_sequence_stamp(tg);

        if(tg->streams[tg->stream_index].testframe_ipv4_udp) {
            frame_udp_checksum_update(tg);
        }
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

