#include <stdint.h>

typedef struct traffic_analyzer_t_ {
    uint64_t totalframes;
    uint64_t testframes;
    struct timespec last_rx_time;
    /* testframe stats */
    struct testframe_ {
        struct timespec last_rx_time;
        struct timespec last_tx_time;
        struct timespec last_latency;
        struct timespec min_latency;
        struct timespec max_latency;
    } testframe;
} traffic_analyzer_t;

traffic_analyzer_t* traffic_analyzer_init(uint32_t frame_size, char* frame_data_hexstr, uint32_t interframe_gap, uint32_t interburst_gap, uint32_t frames_per_burst, uint32_t bursts_per_frame, uint64_t total_frames);
int traffic_analyzer_get_frame(traffic_analyzer_t* tg, uint32_t* frame_length, uint8_t** frame, uint64_t* tx_time_sec, uint32_t* tx_time_nsec);
void traffic_analyzer_put_frame(traffic_analyzer_t* ta, uint8_t* frame_data, uint32_t frame_len, uint64_t rx_sec, uint32_t rx_nsec);

