#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "libtraffic-generator.h"

typedef struct tg_ {
    char* config;
    uint64_t index;
    uint64_t next_tx_time_sec;
    uint32_t next_tx_time_nsec;
} tg_t;

traffic_generator_t traffic_generator_init(const char* config_str, uint32_t* max_frame_size)
{
    tg_t* tg;
    tg = (tg_t*)malloc(sizeof(tg_t));
    memset(tg,0,sizeof(tg_t));
    tg->config=strdup(config_str);
    return (traffic_generator_t)tg;
}
#define PACKET_LEN 64
#define INTERFRAME_GAP 64
int traffic_generator_get_frame(traffic_generator_t tg_id, uint32_t* len, uint8_t* data, uint64_t* tx_time_sec, uint32_t* tx_time_nsec)
{
    uint8_t* data_buf;
    tg_t* tg;
    tg = (tg_t*)tg_id;
    memset(data,0,PACKET_LEN);
    *len=PACKET_LEN;
    *tx_time_sec=tg->next_tx_time_sec;
    *tx_time_nsec=tg->next_tx_time_nsec;
    if(1000000000<(tg->next_tx_time_nsec+8*(PACKET_LEN+INTERFRAME_GAP))) {
        tg->next_tx_time_nsec=(tg->next_tx_time_nsec+8*(PACKET_LEN+INTERFRAME_GAP))%1000000000;
        tg->next_tx_time_sec++;
    } else {
        tg->next_tx_time_nsec+=8*(PACKET_LEN+INTERFRAME_GAP);
    }
    tg->index++;

    return 0;
}
