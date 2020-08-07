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

traffic_generator_t* traffic_generator_init(uint32_t frame_size, char* frame_data_hexstr, uint32_t interframe_gap, uint32_t interburst_gap, uint32_t frames_per_burst, uint32_t bursts_per_stream, uint64_t total_frames, int testframe)
{
    unsigned int i;
    traffic_generator_t* tg;
    tg = (traffic_generator_t*)malloc(sizeof(traffic_generator_t));
    memset(tg,0,sizeof(traffic_generator_t));

    tg->streams_num = 1;
    tg->streams = malloc(sizeof(stream_t)*tg->streams_num);
    memset(tg->streams,0,sizeof(stream_t)*tg->streams_num);

    tg->ns_per_octet = 8.0; /* hardcode to 1Gb */
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
        tg->streams[i].testframe=testframe;
    }

    return tg;
}

void frame_timestamp(traffic_generator_t* tg)
{
    uint8_t* timestamp;
    unsigned int offset;
    offset = tg->streams[tg->stream_index].frame_size - 10;
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

int traffic_generator_get_frame(traffic_generator_t* tg, uint32_t* frame_size, uint8_t** frame_data, uint64_t* tx_time_sec, uint32_t* tx_time_nsec)
{
    struct timespec start, delta, next;

    if((tg->total_frames>0) && ((tg->total_frame_index+1)==tg->total_frames)) {
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

    /* update time and indexes for next step */
    if((tg->frame_index+1) < tg->streams[tg->stream_index].frames_per_burst) {
        tg->frame_index++;
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_size+tg->streams[tg->stream_index].interframe_gap)*tg->ns_per_octet;
    } else if((tg->burst_index+1) < tg->streams[tg->stream_index].bursts_per_stream) {
        tg->frame_index=0;
        tg->burst_index++;
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_size+tg->streams[tg->stream_index].interburst_gap)*tg->ns_per_octet;
    } else {
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_size+tg->streams[tg->stream_index].interstream_gap)*tg->ns_per_octet;
        tg->frame_index=0;
        tg->burst_index=0;
        tg->stream_index=(tg->stream_index+1)%tg->streams_num;
    }
    timespec_add(&start, &delta, &next);

    if(tg->streams[tg->stream_index].testframe) {
        frame_timestamp(tg);
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

