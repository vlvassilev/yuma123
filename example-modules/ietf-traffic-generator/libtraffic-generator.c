#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libtraffic-generator.h"
#include "timespec_math.h"

traffic_generator_t* traffic_generator_init(const char* config_str)
{
    unsigned int i,j,k;
    traffic_generator_t* tg;
    tg = (traffic_generator_t*)malloc(sizeof(traffic_generator_t));
    memset(tg,0,sizeof(traffic_generator_t));

    tg->streams_num = 1;
    tg->streams = malloc(sizeof(stream_t)*tg->streams_num);
    memset(tg->streams,0,sizeof(stream_t)*tg->streams_num);

    tg->ns_per_octet = 8.0; /* hardcode to 1Gb */
    tg->total_frames = 7440476; /* hardcode to 10 sec at 50% rate with minimum packets 10*(1000000000/((20+64)*8))/2=7440476 */
    tg->total_frame_index = 0;

    for(i=0;i<tg->streams_num;i++) {
        tg->streams[i].bursts_per_stream = 1;
        tg->streams[i].bursts = malloc(sizeof(burst_t)*tg->streams[i].bursts_per_stream);
        memset(tg->streams[i].bursts,0,sizeof(burst_t)*tg->streams[i].bursts_per_stream);
        for(j=0;j<tg->streams[i].bursts_per_stream;j++) {
            tg->streams[i].bursts[j].frame_length = 64;
            tg->streams[i].bursts[j].raw_frame_data = malloc(tg->streams[i].bursts[j].frame_length);
            for(k=0;k<tg->streams[i].bursts[j].frame_length;k++) {
                tg->streams[i].bursts[j].raw_frame_data[k]=(uint8_t)k;
            }
            /*init data*/
            tg->streams[i].bursts[j].interframe_gap=20;
            tg->streams[i].bursts[j].frames_per_burst=10;
            tg->streams[i].bursts[j].interburst_gap=(tg->streams[i].bursts[j].interframe_gap+tg->streams[i].bursts[j].frame_length)*tg->streams[i].bursts[j].frames_per_burst; /*50%*/
        }
    }

    return tg;
}

int traffic_generator_get_frame(traffic_generator_t* tg, uint32_t* frame_length, uint8_t** raw_frame_data, uint64_t* tx_time_sec, uint32_t* tx_time_nsec)
{
    struct timespec start, delta, next;

    if((tg->total_frames>0) && ((tg->total_frame_index+1)==tg->total_frames)) {
        return 1;
    } else {
        tg->total_frame_index++;
    }

    *raw_frame_data = tg->streams[tg->stream_index].bursts[tg->burst_index].raw_frame_data;
    *frame_length = tg->streams[tg->stream_index].bursts[tg->burst_index].frame_length;
    *tx_time_sec = tg->sec;
    *tx_time_sec = tg->nsec;

    start.tv_sec = tg->sec;
    start.tv_nsec = tg->nsec;

    /* update time and indexes for next step */
    if((tg->frame_index+1) < tg->streams[tg->stream_index].bursts[tg->burst_index].frames_per_burst) {
        tg->frame_index++;
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_length+tg->streams[tg->stream_index].bursts[tg->burst_index].interframe_gap)*tg->ns_per_octet;
    } else if((tg->burst_index+1) < tg->streams[tg->stream_index].bursts_per_stream) {
        tg->frame_index=0;
        tg->burst_index++;
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_length+tg->streams[tg->stream_index].bursts[tg->burst_index].interburst_gap)*tg->ns_per_octet;
    } else {
        delta.tv_sec=0;
        delta.tv_nsec=(*frame_length+tg->streams[tg->stream_index].interstream_gap)*tg->ns_per_octet;
        tg->frame_index=0;
        tg->burst_index=0;
        tg->stream_index=(tg->stream_index+1)%tg->streams_num;
    }
    timespec_add(&start, &delta, &next);

    tg->sec = next.tv_sec;
    tg->nsec = next.tv_nsec;

    return 0;
}
